# -*- coding: utf-8 -*-

from collections.abc import Iterable
from typing          import Any, BinaryIO

import av, numpy
from native import KeyFinder, PitchShifter, SSTEncoder, WaveformEncoder
from numpy  import dtype, ndarray

## Pitch shifting and .sst ADPCM encoding

NUM_CHANNELS:       int = 2
BLOCKS_PER_SECTOR:  int = 21
SAMPLES_PER_SECTOR: int = 22 * BLOCKS_PER_SECTOR

class VariantEncoder:
	def __init__(self):
		self._encoders: list[SSTEncoder] = [
			SSTEncoder() for _ in range(NUM_CHANNELS)
		]
		self._buffered: ndarray = \
			numpy.empty(( NUM_CHANNELS, 0 ), numpy.float32)

	def _encode(self, samples: ndarray) -> bytes:
		samples           = (samples * 32768.0).clip(-32768.0, 32767.0)
		sector: bytearray = bytearray()

		for channel, encoder in zip(samples, self._encoders):
			sector += encoder.encode(channel.astype(numpy.int16))

		return bytes(sector)

	def feed(
		self,
		samples: ndarray[Any, dtype[numpy.float32]],
		final:   bool = False
	):
		self._buffered = numpy.c_[self._buffered, samples]

	def encodeSector(self) -> bytes:
		samples: ndarray = self._buffered[:, 0:SAMPLES_PER_SECTOR]
		self._buffered   = self._buffered[:, SAMPLES_PER_SECTOR:]

		return self._encode(samples)

	@property
	def availableSectors(self) -> int:
		return self._buffered.shape[1] // SAMPLES_PER_SECTOR

class PitchShiftedVariantEncoder(VariantEncoder):
	def __init__(self, sampleRate: int, pitchOffset: float):
		super().__init__()

		self._shifter: PitchShifter = PitchShifter(
			sampleRate,
			NUM_CHANNELS,
			1.0,
			2.0 ** (pitchOffset / 12.0),
			0x4000
		)

	def feed(
		self,
		samples: ndarray[Any, dtype[numpy.float32]],
		final:   bool = False
	):
		self._shifter.feed(samples, final)

	def encodeSector(self) -> bytes:
		return self._encode(self._shifter.retrieve(SAMPLES_PER_SECTOR))

	@property
	def availableSectors(self) -> int:
		return self._shifter.availableSamples // SAMPLES_PER_SECTOR

## .sst encoder pipeline

class EncodingPipeline:
	def __init__(self, sampleRate: int, pitchOffsets: Iterable[float]):
		self._resampler: av.AudioResampler = av.AudioResampler(
			"fltp",
			"stereo",
			sampleRate,
			SAMPLES_PER_SECTOR
		)
		self._keyFinder: KeyFinder = KeyFinder(
			sampleRate,
			NUM_CHANNELS
		)
		self._variants: list[VariantEncoder] = []
		self._waveform: WaveformEncoder      = WaveformEncoder()

		for pitch in pitchOffsets:
			if (pitch > -0.01) and (pitch < 0.01):
				encoder: VariantEncoder = VariantEncoder()
			else:
				encoder: VariantEncoder = PitchShiftedVariantEncoder(
					sampleRate,
					pitch
				)

			self._variants.append(encoder)

		self.waveformData:  bytearray = bytearray()
		self.chunksEncoded: int       = 0

	def feed(self, frame: av.AudioFrame | None):
		newFrames: list[av.AudioFrame] = self._resampler.resample(frame)

		for newFrame in newFrames:
			samples: ndarray = newFrame.to_ndarray()
			final:   bool    = (frame is None) and (newFrame is newFrames[-1])

			self._keyFinder.feed(samples)

			converted: ndarray = samples.mean(0)
			converted          = (converted * 32768.0).clip(-32768.0, 32767.0)
			self.waveformData += self._waveform.encode(
				converted.astype(numpy.int16),
				self._resampler.rate
			)

			for variant in self._variants:
				variant.feed(samples, final)

	def flush(self, outputFile: BinaryIO):
		while True:
			# Keep flushing as long as at least one sector is available from
			# each variant encoder.
			for variant in self._variants:
				if not variant.availableSectors:
					return

			for variant in self._variants:
				outputFile.write(variant.encodeSector())

			self.chunksEncoded += 1

	def estimateKey(self) -> tuple[str | None, int]:
		return self._keyFinder.estimateKey(True)
