# -*- coding: utf-8 -*-
# distutils: language=c++

from libc.stddef cimport size_t
from libc.stdint cimport int16_t, uint8_t

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
			SSTChunkBase  &output,
			const int16_t *input,
			size_t        numSamples,
			size_t        inputStride
		)

	size_t decodeSST(
		int16_t            *output,
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
			int16_t        *output,
			const BRRBlock *input,
			size_t         numBlocks,
			size_t         outputStride,
			size_t         inputStride
		)
