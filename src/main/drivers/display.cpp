
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/display.hpp"
#include "src/main/drivers/displaydefs.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace drivers {

static const char TAG_[]{ "display" };

/* ST7735 initialization sequence */

/*
 * Based on the initialization sequences found in the following libraries:
 * https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/Adafruit_ST7735.cpp
 * https://github.com/moononournation/Arduino_GFX/blob/master/src/display/Arduino_ST7735.h
 * https://github.com/cnlohr/ch32fun/blob/master/examples/color_lcd/spi_lcd.inl
 */

struct InitCommand {
public:
	uint8_t       command, length, delay;
	const uint8_t *args;
};

static const InitCommand ST7735_INIT_[]{
	{
		.command = ST7735_SWRESET,
		.length  = 0,
		.delay   = 150,
		.args    = nullptr
	}, {
		.command = ST7735_SLPOUT,
		.length  = 0,
		.delay   = 150,
		.args    = nullptr
	}, {
		.command = ST7735_COLMOD,
		.length  = 1,
		.delay   = 10,
		.args    = (const uint8_t[]) { ST7735_COLMOD_IFPF_16BPP }
	}, {
		.command = ST7735_MADCTL,
		.length  = 1,
		.delay   = 10,
		.args    = (const uint8_t[]) {
			/*
			 * Pixels on the TFT module are laid out as follows:
			 *
			 *      G2 --> G161
			 * +-------------------+
			 * |B B B B B B B B B B|
			 * |G G G G G G G G G G|  S7
			 * |R R R R R R R R R R|  v
			 * |        ...        | S390
			 * |R R R R R R R R R R|
			 * +-------------------+
			 */
			0
				| ST7735_MADCTL_MH_RIGHT
				| ST7735_MADCTL_ORDER_RGB
				| ST7735_MADCTL_ML_DOWN
				| ST7735_MADCTL_MV_COLUMN
				| ST7735_MADCTL_MX_LEFT
				| ST7735_MADCTL_MY_DOWN
		}
#if 0
	}, {
		.command = ST7735_FRMCTR1,
		.length  = 3,
		.delay   = 0,
		.args    = (const uint8_t[]) { 0x01, 0x2c, 0x2d }
	}, {
		.command = ST7735_FRMCTR2,
		.length  = 3,
		.delay   = 0,
		.args    = (const uint8_t[]) { 0x01, 0x2c, 0x2d }
	}, {
		.command = ST7735_FRMCTR3,
		.length  = 6,
		.delay   = 10,
		.args    = (const uint8_t[]) { 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d }
#endif
	}, {
		.command = ST7735_GAMCTRP1,
		.length  = 16,
		.delay   = 0,
		.args    = (const uint8_t[]) {
#if 1
			0x09, 0x16, 0x09, 0x20,
			0x21, 0x1b, 0x13, 0x19,
			0x17, 0x15, 0x1e, 0x2b,
			0x04, 0x05, 0x02, 0x0e
#else
			0x02, 0x1c, 0x07, 0x12,
			0x37, 0x32, 0x29, 0x2d,
			0x29, 0x25, 0x2b, 0x39,
			0x00, 0x01, 0x03, 0x10,
#endif
		}
	}, {
		.command = ST7735_GAMCTRN1,
		.length  = 16,
		.delay   = 10,
		.args    = (const uint8_t[]) {
#if 1
			0x0b, 0x14, 0x08, 0x1e,
			0x22, 0x1d, 0x18, 0x1e,
			0x1b, 0x1a, 0x24, 0x2b,
			0x06, 0x06, 0x02, 0x0f
#else
			0x03, 0x1d, 0x07, 0x06,
			0x2e, 0x2c, 0x29, 0x2d,
			0x2e, 0x2e, 0x37, 0x3f,
			0x00, 0x00, 0x02, 0x10
#endif
		}
	}, {
		.command = ST7735_INVOFF,
		.length  = 0,
		.delay   = 10,
		.args    = nullptr
	}, {
		.command = ST7735_NORON,
		.length  = 0,
		.delay   = 10,
		.args    = nullptr
	}, {
		.command = ST7735_DISPON,
		.length  = 0,
		.delay   = 10,
		.args    = nullptr
	}
};

/* Transaction packing */

static void makeCommandPacket_(spi_transaction_t &output, uint8_t command) {
	output.flags      = SPI_TRANS_USE_TXDATA;
	output.cmd        = 0;
	output.addr       = 0;
	output.length     = 8;
	output.rxlength   = 0;
	output.user       = reinterpret_cast<void *>(0);
	output.tx_data[0] = command;
	output.rx_buffer  = nullptr;
}

static void makeAddressPacket_(
	spi_transaction_t &output,
	uint16_t          start,
	uint16_t          end
) {
	output.flags      = SPI_TRANS_USE_TXDATA;
	output.cmd        = 0;
	output.addr       = 0;
	output.length     = 4 * 8;
	output.rxlength   = 0;
	output.user       = reinterpret_cast<void *>(1);
	output.tx_data[0] = start >> 8;
	output.tx_data[1] = start & 0xff;
	output.tx_data[2] = end   >> 8;
	output.tx_data[3] = end   & 0xff;
	output.rx_buffer  = nullptr;
}

static void makeDataPacket_(
	spi_transaction_t &output,
	const void        *data,
	size_t            length
) {
	output.flags     = 0;
	output.cmd       = 0;
	output.addr      = 0;
	output.length    = length * 8;
	output.rxlength  = 0;
	output.user      = reinterpret_cast<void *>(1);
	output.tx_buffer = data;
	output.rx_buffer = nullptr;
}

/* Display manager class */

static constexpr size_t MAX_WIDTH_  = 162;
static constexpr size_t MAX_HEIGHT_ = 132;

// Each full display update requires 13 queued transactions, containing:
// - the CASET command (D/C = 0);
// - the start and end column indices (D/C = 1);
// - the RASET command (D/C = 0);
// - the start and end row indices (D/C = 1);
// - the RAMWR command (D/C = 0);
// - 8 packets holding 16 scanlines each (D/C = 1).
static constexpr size_t LINES_PER_TRANSFER_ = 16;
static constexpr size_t BYTES_PER_TRANSFER_ =
	MAX_WIDTH_ * LINES_PER_TRANSFER_ * sizeof(uint16_t);
static constexpr size_t QUEUE_DEPTH_        = 0
	+ 5
	+ (MAX_HEIGHT_ + LINES_PER_TRANSFER_ - 1) / LINES_PER_TRANSFER_;

static constexpr int SPI_BAUD_RATE_ = 8000000;
static constexpr int PWM_FREQUENCY_ = 50000;

static constexpr int BACKLIGHT_BITS_ = 8;
static constexpr int BACKLIGHT_UNIT_ = 1 << BACKLIGHT_BITS_;

static const spi_bus_config_t BUS_CONFIG_{
	.mosi_io_num           = defs::IO_DISPLAY_SDA,
	.miso_io_num           = GPIO_NUM_NC,
	.sclk_io_num           = defs::IO_DISPLAY_SCL,
	.data2_io_num          = GPIO_NUM_NC,
	.data3_io_num          = GPIO_NUM_NC,
	.data4_io_num          = GPIO_NUM_NC,
	.data5_io_num          = GPIO_NUM_NC,
	.data6_io_num          = GPIO_NUM_NC,
	.data7_io_num          = GPIO_NUM_NC,
	.data_io_default_level = false,
	.max_transfer_sz       = BYTES_PER_TRANSFER_,
	.flags                 = 0
		| SPICOMMON_BUSFLAG_MASTER
		| SPICOMMON_BUSFLAG_IOMUX_PINS,
	.isr_cpu_id            = ESP_INTR_CPU_AFFINITY_AUTO,
	.intr_flags            = 0
};

static const spi_device_interface_config_t DEVICE_CONFIG_{
	.command_bits     = 0,
	.address_bits     = 0,
	.dummy_bits       = 0,
	.mode             = 0, // CPOL=0, CPHA=0
	.clock_source     = SPI_CLK_SRC_DEFAULT,
	.duty_cycle_pos   = 128,
	.cs_ena_pretrans  = 0,
	.cs_ena_posttrans = 0,
	.clock_speed_hz   = SPI_BAUD_RATE_,
	.input_delay_ns   = 0,
	.sample_point     = SPI_SAMPLING_POINT_PHASE_0,
	.spics_io_num     = defs::IO_DISPLAY_CS,
	.flags            = SPI_DEVICE_3WIRE,
	.queue_size       = QUEUE_DEPTH_ + 1,
	.pre_cb           = [](spi_transaction_t *transaction) IRAM_ATTR {
		// Update D/C asynchronously once the transaction is about to run.
		gpio_set_level(gpio_num_t(defs::IO_DISPLAY_D_C), int(transaction->user));
	},
	.post_cb          = nullptr
};

static const gpio_config_t GPIO_CONFIG_{
	.pin_bit_mask = uint64_t(1) << defs::IO_DISPLAY_D_C,
	.mode         = GPIO_MODE_OUTPUT,
	.pull_up_en   = GPIO_PULLUP_DISABLE,
	.pull_down_en = GPIO_PULLDOWN_DISABLE,
	.intr_type    = GPIO_INTR_DISABLE
};

static const ledc_timer_config_t BACKLIGHT_TIMER_CONFIG_{
	.speed_mode      = LEDC_LOW_SPEED_MODE,
	.duty_resolution = ledc_timer_bit_t(BACKLIGHT_BITS_),
	.timer_num       = defs::DISPLAY_LEDC_TIMER,
	.freq_hz         = PWM_FREQUENCY_,
	.clk_cfg         = LEDC_AUTO_CLK,
	.deconfigure     = false
};

static const ledc_channel_config_t BACKLIGHT_CHANNEL_CONFIG_{
	.gpio_num   = defs::IO_DISPLAY_BL,
	.speed_mode = LEDC_LOW_SPEED_MODE,
	.channel    = defs::DISPLAY_LEDC_CHANNEL,
	.intr_type  = LEDC_INTR_DISABLE,
	.timer_sel  = defs::DISPLAY_LEDC_TIMER,
	.duty       = 0,
	.hpoint     = 0,
	.sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
	.flags      = {
		.output_invert = false
	}
};

DisplayDriver &DisplayDriver::instance(void) {
	static DisplayDriver driver;

	return driver;
}

void DisplayDriver::init(int width, int height) {
	assert((width  > 0) && (width  <= MAX_WIDTH_));
	assert((height > 0) && (height <= MAX_HEIGHT_));

	if (device_)
		release();

	// Set up the SPI bus and run the display initialization sequence.
	spi_bus_initialize(defs::DISPLAY_SPI_HOST, &BUS_CONFIG_, SPI_DMA_CH_AUTO);
	spi_bus_add_device(defs::DISPLAY_SPI_HOST, &DEVICE_CONFIG_, &device_);
	gpio_config(&GPIO_CONFIG_);

	for (auto &entry : ST7735_INIT_) {
		spi_transaction_t transaction;

		makeCommandPacket_(transaction, entry.command);
		spi_device_polling_transmit(device_, &transaction);

		if (entry.length) {
			makeDataPacket_(transaction, entry.args, entry.length);
			spi_device_polling_transmit(device_, &transaction);
		}

		vTaskDelay(entry.delay / portTICK_PERIOD_MS);
	}

	// Set up the backlight.
	ledc_timer_config(&BACKLIGHT_TIMER_CONFIG_);
	ledc_channel_config(&BACKLIGHT_CHANNEL_CONFIG_);
	setBacklight(1.0f);

	// Allocate and initialize the transactions used to update the display
	// asynchronously.
	asyncTransactions_.allocate<spi_transaction_t>(QUEUE_DEPTH_);
	assert(asyncTransactions_.ptr);
	memset(asyncTransactions_.ptr, 0, asyncTransactions_.length);

	auto transactions = asyncTransactions_.as<spi_transaction_t>();

	makeCommandPacket_(transactions[0], ST7735_CASET);
	makeCommandPacket_(transactions[2], ST7735_RASET);
	makeCommandPacket_(transactions[4], ST7735_RAMWR);
}

void DisplayDriver::release(void) {
	if (!device_)
		return;

	setBacklight(0.0f);
	spi_bus_remove_device(device_);
	spi_bus_free(defs::DISPLAY_SPI_HOST);

	device_ = nullptr;
	asyncTransactions_.destroy();
}

bool DisplayDriver::updateAsync(
	int            x,
	int            y,
	int            width,
	int            height,
	const uint16_t *data
) {
	assert(device_);

	// Update the start and end addresses, then queue a transaction for each
	// block of 16 lines.
	auto transactions    = asyncTransactions_.as<spi_transaction_t>();
	int  numTransactions = 5;

	makeAddressPacket_(transactions[1], x, x + width  - 1);
	makeAddressPacket_(transactions[3], y, y + height - 1);

	while (height > 0) {
		const int transferHeight = util::min<int>(height, LINES_PER_TRANSFER_);

		makeDataPacket_(
			transactions[numTransactions++],
			data,
			width * transferHeight * sizeof(uint16_t)
		);
		assert(numTransactions <= QUEUE_DEPTH_);

		height -= transferHeight;
		data   += width * transferHeight;
	}

	for (int i = 0; i < numTransactions; i++) {
		if (spi_device_queue_trans(
			device_,
			&transactions[i],
			portMAX_DELAY
		) != ESP_OK) {
			ESP_LOGE(TAG_, "failed to queue packet %d", i);
			return false;
		}
	}

	return true;
}

void DisplayDriver::setBacklight(float brightness) {
	assert(device_);

	const int value = int(brightness * float(BACKLIGHT_UNIT_) + 0.5f);

	ledc_set_duty(
		LEDC_LOW_SPEED_MODE,
		defs::DISPLAY_LEDC_CHANNEL,
		util::clamp(value, 0, BACKLIGHT_UNIT_ - 1)
	);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, defs::DISPLAY_LEDC_CHANNEL);
}

}
