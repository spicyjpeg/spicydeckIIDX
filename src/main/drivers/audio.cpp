
#include <assert.h>
#include <stddef.h>
#include "driver/i2s_common.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/audio.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace drivers {

static const char TAG_[]{ "audio" };

/* Audio output manager class */

static constexpr size_t QUEUE_DEPTH_ = 4;

static const i2s_std_slot_config_t SLOT_CONFIG_{
	.data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
	.slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
	.slot_mode      = I2S_SLOT_MODE_STEREO,
	.slot_mask      = I2S_STD_SLOT_BOTH,
	.ws_width       = I2S_DATA_BIT_WIDTH_16BIT,
	.ws_pol         = false,
	.bit_shift      = true,
	.msb_right      = true
};

static const i2s_std_gpio_config_t MAIN_GPIO_CONFIG_{
	.mclk         = I2S_GPIO_UNUSED,
	.bclk         = gpio_num_t(defs::IO_I2S_BCLK),
	.ws           = gpio_num_t(defs::IO_I2S_LRCK),
	.dout         = gpio_num_t(defs::IO_I2S_SDOUT0),
	.din          = I2S_GPIO_UNUSED,
	.invert_flags = {
		.mclk_inv = false,
		.bclk_inv = false,
		.ws_inv   = false
	}
};

static const i2s_std_gpio_config_t MONITOR_GPIO_CONFIG_{
	.mclk         = I2S_GPIO_UNUSED,
	.bclk         = gpio_num_t(defs::IO_I2S_BCLK),
	.ws           = gpio_num_t(defs::IO_I2S_LRCK),
	.dout         = gpio_num_t(defs::IO_I2S_SDOUT1),
	.din          = I2S_GPIO_UNUSED,
	.invert_flags = {
		.mclk_inv = false,
		.bclk_inv = false,
		.ws_inv   = false
	}
};

AudioDriver &AudioDriver::instance(void) {
	static AudioDriver driver;

	return driver;
}

void AudioDriver::init(int sampleRate, size_t samplesPerBuffer) {
	if (main_)
		release();

	// Configure I2S0 as master and I2S1 as slave. This allows for perfect
	// synchronization as BCLK and LRCK from I2S0 will backfeed I2S1 through the
	// GPIO matrix.
	i2s_chan_config_t channelConfig;

	channelConfig.dma_desc_num         = QUEUE_DEPTH_;
	channelConfig.dma_frame_num        = samplesPerBuffer;
	channelConfig.auto_clear_after_cb  = false;
	channelConfig.auto_clear_before_cb = false;
	channelConfig.allow_pd             = false;
	channelConfig.intr_priority        = 0;

	channelConfig.id   = defs::MAIN_I2S_PORT;
	channelConfig.role = I2S_ROLE_MASTER;
	i2s_new_channel(&channelConfig, &main_, nullptr);
	channelConfig.id   = defs::MONITOR_I2S_PORT;
	channelConfig.role = I2S_ROLE_SLAVE;
	i2s_new_channel(&channelConfig, &monitor_, nullptr);

	i2s_std_config_t stdConfig;

	stdConfig.clk_cfg.sample_rate_hz = sampleRate;
	stdConfig.clk_cfg.clk_src        = I2S_CLK_SRC_APLL;
	stdConfig.clk_cfg.mclk_multiple  = I2S_MCLK_MULTIPLE_256;
	util::copy(stdConfig.slot_cfg, SLOT_CONFIG_);

	util::copy(stdConfig.gpio_cfg, MAIN_GPIO_CONFIG_);
	i2s_channel_init_std_mode(main_, &stdConfig);
	util::copy(stdConfig.gpio_cfg, MONITOR_GPIO_CONFIG_);
	i2s_channel_init_std_mode(monitor_, &stdConfig);

	i2s_channel_enable(main_);
	i2s_channel_enable(monitor_);
}

void AudioDriver::release(void) {
	if (!main_)
		return;

	i2s_channel_disable(main_);
	i2s_channel_disable(monitor_);
	i2s_del_channel(main_);
	i2s_del_channel(monitor_);
	main_    = nullptr;
	monitor_ = nullptr;
}

size_t AudioDriver::feed(
	const Sample *main,
	const Sample *monitor,
	size_t       numSamples
) {
	assert(main_);

	const size_t length = numSamples * sizeof(Sample) * 2;
	size_t       actualLength;

	if (i2s_channel_write(
		main_,
		main,
		length,
		&actualLength,
		portMAX_DELAY
	) != ESP_OK) {
		ESP_LOGE(TAG_, "main I2S write failed");
		return 0;
	}
	if (i2s_channel_write(
		monitor_,
		monitor,
		length,
		&actualLength,
		portMAX_DELAY
	) != ESP_OK) {
		ESP_LOGE(TAG_, "monitor I2S write failed");
		return 0;
	}

	return actualLength / (sizeof(Sample) * 2);
}

}
