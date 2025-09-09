# -*- coding: utf-8 -*-
# distutils: language=c++

from libc.stddef cimport size_t
from libc.stdint cimport int16_t, uint8_t

ctypedef int16_t Sample

cdef extern from "src/main/dsp/adpcm.hpp" namespace "dsp":
	# 12-byte .sst ADPCM encoder and decoder

	cdef struct SSTBlock:
		uint8_t header
		uint8_t samples[11]

	cdef struct SSTChunkBase:
		int16_t  s1, s2
		SSTBlock blocks[]

	cdef const size_t SST_SAMPLES_PER_BLOCK = 22

	cdef cppclass SSTEncoder:
		SSTEncoder()

		void reset()
		size_t encode(
			SSTChunkBase &output,
			const Sample *input,
			size_t       numSamples,
			size_t       inputStride
		)

	size_t decodeSST(
		Sample             *output,
		const SSTChunkBase &input,
		size_t             numBlocks,
		size_t             outputStride
	)

	# 16-byte BRR ADPCM decoder (unused)

	cdef struct BRRBlock:
		uint8_t header, loopFlags
		uint8_t samples[14]

	cdef const size_t BRR_SAMPLES_PER_BLOCK = 28

	cdef cppclass BRRDecoder:
		BRRDecoder()

		void reset()
		size_t decode(
			Sample         *output,
			const BRRBlock *input,
			size_t         numBlocks,
			size_t         outputStride,
			size_t         inputStride
		)

cdef extern from "src/main/dsp/dsp.hpp" namespace "dsp":
	# 4-bit waveform data generator

	cdef const int     WAVEFORM_SAMPLE_RATE = 32
	cdef const uint8_t WAVEFORM_RANGE       = 12

	cdef cppclass WaveformEncoder:
		WaveformEncoder()

		void reset()
		size_t encode(
			uint8_t      *output,
			const Sample *input,
			int          sampleRate,
			size_t       numSamples,
			size_t       inputStride
		)
