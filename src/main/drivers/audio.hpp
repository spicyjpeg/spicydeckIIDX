
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "driver/i2s_types.h"

namespace drivers {

using Sample = int16_t;

/* Audio output manager class */

class AudioDriver {
private:
	i2s_chan_handle_t main_, monitor_;

	inline AudioDriver(void) :
		main_(nullptr)
	{}
	inline ~AudioDriver(void) {
		release();
	}

public:
	static AudioDriver &instance(void);
	void init(int sampleRate, size_t samplesPerBuffer);
	void release(void);

	size_t feed(const Sample *main, const Sample *monitor, size_t numSamples);
};

}
