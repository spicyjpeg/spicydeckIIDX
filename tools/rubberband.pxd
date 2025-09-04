# -*- coding: utf-8 -*-
# distutils: language=c++

from libc.stddef cimport size_t
from libcpp      cimport bool

cdef enum Option:
	OptionProcessOffline       = 0 <<  0
	OptionProcessRealTime      = 1 <<  0
	OptionTransientsCrisp      = 0 <<  8
	OptionTransientsMixed      = 1 <<  8
	OptionTransientsSmooth     = 2 <<  8
	OptionDetectorCompound     = 0 << 10
	OptionDetectorPercussive   = 1 << 10
	OptionDetectorSoft         = 2 << 10
	OptionPhaseLaminar         = 0 << 13
	OptionPhaseIndependent     = 1 << 13
	OptionThreadingAuto        = 0 << 16
	OptionThreadingNever       = 1 << 16
	OptionThreadingAlways      = 2 << 16
	OptionWindowStandard       = 0 << 20
	OptionWindowShort          = 1 << 20
	OptionWindowLong           = 2 << 20
	OptionSmoothingOff         = 0 << 23
	OptionSmoothingOn          = 1 << 23
	OptionFormantShifted       = 0 << 24
	OptionFormantPreserved     = 1 << 24
	OptionPitchHighSpeed       = 0 << 25
	OptionPitchHighQuality     = 1 << 25
	OptionPitchHighConsistency = 2 << 25
	OptionChannelsApart        = 0 << 28
	OptionChannelsTogether     = 1 << 28
	OptionEngineFaster         = 0 << 29
	OptionEngineFiner          = 1 << 29

cdef enum PresetOption:
	DefaultOptions    = 0
	PercussiveOptions = (1 << 13) | (1 << 20)

cdef extern from "rubberband/RubberBandStretcher.h" namespace "RubberBand":
	cdef cppclass RubberBandStretcher:
		RubberBandStretcher(
			size_t sampleRate,
			size_t channels,
			int    options,
			double initialTimeRatio,
			double initialPitchScale
		)

		void reset()
		int getEngineVersion() const

		void setTimeRatio(double ratio)
		void setPitchScale(double scale)
		void setFormantScale(double scale)
		double getTimeRatio() const
		double getPitchScale() const
		double getFormantScale() const

		size_t getPreferredStartPad() const
		size_t getStartDelay() const
		size_t getChannelCount() const
		void setFormantOption(int options)

		void setExpectedInputDuration(size_t samples)
		void setMaxProcessSize(size_t samples)
		size_t getProcessSizeLimit() const

		size_t getSamplesRequired() const
		void study(const float *const *input, size_t samples, bool final)
		void process(const float *const *input, size_t samples, bool final)

		int available() const
		size_t retrieve(float *const *output, size_t samples) const
