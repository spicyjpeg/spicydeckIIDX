# -*- coding: utf-8 -*-
# distutils: extra_compile_args=[ -O3, -std=gnu++20 ]
# distutils: include_dirs=..
# distutils: language=c++
# distutils: libraries=[ keyfinder, rubberband ]
# distutils: sources=[ ../src/main/dsp/adpcm.cpp, ../src/main/dsp/dsp.cpp ]
# cython:    language_level=3

from cython.operator cimport dereference
from libc.stddef     cimport size_t
from libc.stdint     cimport int16_t, uint8_t
from libcpp          cimport bool

import numpy
from numpy import ndarray

cimport dsp, keyfinder
from dsp        cimport \
	SST_SAMPLES_PER_BLOCK, WAVEFORM_SAMPLE_RATE, SSTBlock, SSTChunkBase
from keyfinder  cimport key_t
from rubberband cimport Option, RubberBandStretcher

## .sst ADPCM encoder/decoder bindings

cdef class SSTEncoder:
	cdef dsp.SSTEncoder _encoder

	def reset(self):
		self._encoder.reset()

	def encode(self, const int16_t[::1] samples not None) -> bytearray:
		if not samples.shape[0]:
			return bytearray()

		cdef size_t numBlocks = samples.shape[0]
		numBlocks            += SST_SAMPLES_PER_BLOCK - 1
		numBlocks           //= SST_SAMPLES_PER_BLOCK
		chunk                 = \
			bytearray(sizeof(SSTChunkBase) + numBlocks * sizeof(SSTBlock))

		cdef uint8_t[::1] chunkView = chunk

		self._encoder.encode(
			dereference(<SSTChunkBase *> &chunkView[0]),
			&samples[0],
			samples.shape[0],
			1
		)

		return chunk

def decodeSST(const uint8_t[::1] chunk not None) -> ndarray:
	cdef size_t dataLength = chunk.shape[0] - sizeof(SSTChunkBase)

	if dataLength < 0:
		raise ValueError("invalid input chunk header")
	if not dataLength:
		return numpy.empty(0, numpy.int16)
	if dataLength % sizeof(SSTBlock):
		raise ValueError(
			"input chunk must consist of a header and block aligned data"
		)

	cdef size_t numBlocks = dataLength // sizeof(SSTBlock)
	samples               = \
		numpy.empty(numBlocks * SST_SAMPLES_PER_BLOCK, numpy.int16)

	cdef int16_t[::1] samplesView = samples

	cdef size_t numDecoded = dsp.decodeSST(
		&samplesView[0],
		dereference(<const SSTChunkBase *> &chunk[0]),
		numBlocks,
		1
	)

	return samples[0:numDecoded]

## Waveform data generator bindings

cdef class WaveformEncoder:
	cdef dsp.WaveformEncoder _encoder

	def reset(self):
		self._encoder.reset()

	def encode(
		self,
		const int16_t[::1] samples not None,
		int                sampleRate
	) -> bytearray:
		if not samples.shape[0]:
			return bytearray()

		cdef size_t numNibbles = samples.shape[0] * sampleRate
		numNibbles            += WAVEFORM_SAMPLE_RATE - 1
		numNibbles           //= WAVEFORM_SAMPLE_RATE
		chunk                  = bytearray((numNibbles + 1) // 2)

		cdef uint8_t[::1] chunkView = chunk

		cdef size_t numEncoded = self._encoder.encode(
			&chunkView[0],
			&samples[0],
			sampleRate,
			samples.shape[0],
			1,
		)

		return chunk[0:(numEncoded + 1) // 2]

## KeyFinder bindings

cdef class KeyFinder:
	cdef int _sampleRate, _numChannels

	cdef keyfinder.KeyFinder _keyFinder
	cdef keyfinder.Workspace _workspace

	def __init__(self, int sampleRate, int numChannels):
		self._sampleRate  = sampleRate
		self._numChannels = numChannels

	def feed(self, const float[:, ::1] samples not None):
		if samples.shape[0] != self._numChannels:
			raise ValueError("invalid channel count")

		cdef keyfinder.AudioData buffer

		buffer.setFrameRate(self._sampleRate)
		buffer.setChannels(self._numChannels)
		buffer.addToFrameCount(samples.shape[1])

		for i in range(self._numChannels):
			for j in range(samples.shape[1]):
				buffer.setSampleByFrame(j, i, samples[i, j])

		self._keyFinder.progressiveChromagram(buffer, self._workspace)

	def estimateKey(self, bool final = False) -> tuple[str | None, int]:
		if final:
			self._keyFinder.finalChromagram(self._workspace)

		cdef int key     = self._keyFinder.keyOfChromagram(self._workspace)
		cdef int keyNote = key // 2

		if key == key_t.SILENCE:
			keyScale = None
		else:
			keyScale = "minor" if (key % 2) else "major"

		return keyScale, (keyNote + 9) % 12

## Rubber Band pitch shifter bindings

cdef class PitchShifter:
	cdef RubberBandStretcher *_stretcher

	def __init__(
		self,
		int   sampleRate,
		int   numChannels,
		float timeRatio       = 1.0,
		float pitchScale      = 1.0,
		int   maxFeedLength   = 0,
		bool  highConsistency = False,
		bool  preserveFormant = False
	):
		if numChannels > 16:
			raise ValueError("unsupported channel count")
		if maxFeedLength > 0x80000:
			raise ValueError("unsupported maximum feeding length")

		cdef int options = (0
			| Option.OptionProcessRealTime
			| Option.OptionChannelsTogether
			| Option.OptionEngineFiner
		)

		if highConsistency:
			options |= Option.OptionPitchHighConsistency
		else:
			options |= Option.OptionPitchHighQuality
		if preserveFormant:
			options |= Option.OptionFormantPreserved

		self._stretcher = new RubberBandStretcher(
			sampleRate,
			numChannels,
			options,
			timeRatio,
			pitchScale
		)

		if maxFeedLength > 0:
			self._stretcher.setMaxProcessSize(maxFeedLength)

	def __dealloc__(self):
		del self._stretcher

	@property
	def numChannels(self) -> int:
		return self._stretcher.getChannelCount()

	@property
	def timeRatio(self) -> float:
		return self._stretcher.getTimeRatio()

	@timeRatio.setter
	def timeRatio(self, float value):
		self._stretcher.setTimeRatio(value)

	@property
	def pitchScale(self) -> float:
		return self._stretcher.getPitchScale()

	@pitchScale.setter
	def pitchScale(self, float value):
		self._stretcher.setPitchScale(value)

	@property
	def formantScale(self) -> float:
		return self._stretcher.getFormantScale()

	@formantScale.setter
	def formantScale(self, float value):
		self._stretcher.setFormantScale(value)

	def reset(self):
		self._stretcher.reset()

	def feed(self, const float[:, ::1] samples not None, bool final = False):
		cdef int numChannels = self._stretcher.getChannelCount()

		if samples.shape[0] != numChannels:
			raise ValueError("invalid channel count")

		cdef const float *buffers[16]

		for i in range(numChannels):
			buffers[i] = &samples[i, 0]

		self._stretcher.process(&buffers[0], samples.shape[1], final)

	def retrieve(self, int maxSamples = 0) -> ndarray:
		cdef int numChannels = self._stretcher.getChannelCount()
		samples              = \
			numpy.empty(( numChannels, maxSamples ), numpy.float32)

		cdef float[:, ::1] samplesView = samples
		cdef float         *buffers[16]

		for i in range(numChannels):
			buffers[i] = &samplesView[i, 0]

		cdef size_t numRetrieved = \
			self._stretcher.retrieve(&buffers[0], maxSamples)

		return samples[:, 0:numRetrieved]

	@property
	def requiredSamples(self) -> int:
		return self._stretcher.getSamplesRequired()

	@property
	def availableSamples(self) -> int:
		return self._stretcher.available()
