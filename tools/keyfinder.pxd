# -*- coding: utf-8 -*-
# distutils: language=c++

from libcpp cimport bool

cdef enum key_t:
	A_MAJOR      =  0
	A_MINOR      =  1
	B_FLAT_MAJOR =  2
	B_FLAT_MINOR =  3
	B_MAJOR      =  4
	B_MINOR      =  5
	C_MAJOR      =  6
	C_MINOR      =  7
	D_FLAT_MAJOR =  8
	D_FLAT_MINOR =  9
	D_MAJOR      = 10
	D_MINOR      = 11
	E_FLAT_MAJOR = 12
	E_FLAT_MINOR = 13
	E_MAJOR      = 14
	E_MINOR      = 15
	F_MAJOR      = 16
	F_MINOR      = 17
	G_FLAT_MAJOR = 18
	G_FLAT_MINOR = 19
	G_MAJOR      = 20
	G_MINOR      = 21
	A_FLAT_MAJOR = 22
	A_FLAT_MINOR = 23
	SILENCE      = 24

cdef extern from "keyfinder/audiodata.h" namespace "KeyFinder":
	cdef cppclass AudioData:
		AudioData()

		unsigned int getChannels() const
		unsigned int getFrameRate() const
		double getSample(unsigned int index) const
		double getSampleByFrame(unsigned int frame, unsigned int channel) const
		double getSampleAtReadIterator() const
		unsigned int getSampleCount() const
		unsigned int getFrameCount() const

		void setChannels(unsigned int newChannels)
		void setFrameRate(unsigned int newFrameRate)
		void setSample(unsigned int index, double value);
		void setSampleByFrame(
			unsigned int frame,
			unsigned int channels,
			double       value
		)
		void setSampleAtWriteIterator(double value)
		void addToSampleCount(unsigned int newSamples)
		void addToFrameCount(unsigned int newFrames)

		void advanceReadIterator(unsigned int by)
		void advanceWriteIterator(unsigned int by)
		bool readIteratorWithinUpperBound() const
		bool writeIteratorWithinUpperBound() const
		void resetIterators()

		void append(const AudioData &that)
		void prepend(const AudioData &that)
		void discardFramesFromFront(unsigned int discardFrameCount)
		void reduceToMono()
		void downsample(unsigned int factor, bool shortcut)
		AudioData *sliceSamplesFromBack(unsigned int sliceSampleCount)

cdef extern from "keyfinder/workspace.h" namespace "KeyFinder":
	cdef cppclass Workspace:
		Workspace()

cdef extern from "keyfinder/keyfinder.h" namespace "KeyFinder":
	cdef cppclass KeyFinder:
		void progressiveChromagram(AudioData audio, Workspace &workspace)
		void finalChromagram(Workspace &workspace)
		key_t keyOfChromagram(const Workspace &workspace) const

		key_t keyOfAudio(const AudioData &audio)
