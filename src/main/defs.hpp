
#pragma once

#ifdef ESP_PLATFORM
#include "driver/i2c_types.h"
#include "driver/i2s_types.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "hal/ledc_types.h"
#include "hal/spi_types.h"
#else
#include <stdio.h>
#endif

namespace defs {

/* ESP-IDF polyfill */

#ifndef ESP_PLATFORM

#define IRAM_ATTR
#define DRAM_ATTR

#define ESP_LOGV(tag, format, ...) \
	fprintf(stderr, "[V] " tag ": " format "\n" __VA_OPT__(,) __VA_ARGS__)
#define ESP_LOGD(tag, format, ...) \
	fprintf(stderr, "[D] " tag ": " format "\n" __VA_OPT__(,) __VA_ARGS__)
#define ESP_LOGI(tag, format, ...) \
	fprintf(stderr, "[I] " tag ": " format "\n" __VA_OPT__(,) __VA_ARGS__)
#define ESP_LOGE(tag, format, ...) \
	fprintf(stderr, "[E] " tag ": " format "\n" __VA_OPT__(,) __VA_ARGS__)
#define ESP_LOGW(tag, format, ...) \
	fprintf(stderr, "[W] " tag ": " format "\n" __VA_OPT__(,) __VA_ARGS__)

#endif

/* GPIO pin and peripheral definitions */

#ifdef ESP_PLATFORM

enum GPIOPin {
	IO_SD_CMD = 15,
	IO_SD_DAT =  2,
	IO_SD_CLK = 14,

	IO_DISPLAY_SDA = 23,
	IO_DISPLAY_SCL = 18,
	IO_DISPLAY_CS  =  5,
	IO_DISPLAY_D_C = 19,
	IO_DISPLAY_BL  =  0,

	IO_I2S_SDOUT0 = 27,
	IO_I2S_SDOUT1 =  4,
	IO_I2S_BCLK   = 12,
	IO_I2S_LRCK   = 13,

	IO_LEFT_DECK_SDA  = 16,
	IO_LEFT_DECK_SCL  = 17,
	IO_RIGHT_DECK_SDA = 21,
	IO_RIGHT_DECK_SCL = 22,

	IO_LEFT_DECK_PWM_A  = 25,
	IO_LEFT_DECK_PWM_B  = 26,
	IO_RIGHT_DECK_PWM_A = 32,
	IO_RIGHT_DECK_PWM_B = 33,

	IO_SELECTOR_A   = 34,
	IO_SELECTOR_B   = 35,
	IO_SELECTOR_BTN = 36
};

static constexpr spi_host_device_t DISPLAY_SPI_HOST = SPI3_HOST;

static constexpr ledc_timer_t   DISPLAY_LEDC_TIMER   = LEDC_TIMER_0;
static constexpr ledc_channel_t DISPLAY_LEDC_CHANNEL = LEDC_CHANNEL_0;

static constexpr i2s_port_t MAIN_I2S_PORT    = I2S_NUM_0;
static constexpr i2s_port_t MONITOR_I2S_PORT = I2S_NUM_1;

static constexpr i2c_port_num_t LEFT_DECK_I2C_PORT  = I2C_NUM_0;
static constexpr i2c_port_num_t RIGHT_DECK_I2C_PORT = I2C_NUM_1;

static constexpr int DECK_MCPWM_GROUP = 0;

#endif

}
