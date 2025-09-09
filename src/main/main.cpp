
#include "esp_heap_caps.h"
#include "src/main/drivers/audio.hpp"
#include "src/main/drivers/display.hpp"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/motor.hpp"
#include "src/main/drivers/storage.hpp"
#include "src/main/renderer/font.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/audiotask.hpp"
#include "src/main/defs.hpp"
#include "src/main/iotask.hpp"
#include "src/main/uitask.hpp"

static const char TAG_[]{ "main" };

/* Entry point */

static void showErrorScreen(const char *text) {
	auto &display = drivers::DisplayDriver::instance();

	renderer::Renderer gfx;
	renderer::Font     font;

	gfx.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	gfx.clear(UI_COLOR_BACKGROUND);

	font.initDefault();
	font.draw(
		gfx,
		8,
		8,
		DISPLAY_WIDTH - (8 * 2),
		16,
		"Error",
		UI_COLOR_TITLE
	);
	font.draw(
		gfx,
		8,
		8 + 16,
		DISPLAY_WIDTH  - (8 * 2),
		DISPLAY_HEIGHT - (8 * 2 + 16),
		text,
		UI_COLOR_TEXT1,
		true
	);

	display.updateAsync(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, gfx.flip());
}

void run(void) {
	auto &display = drivers::DisplayDriver::instance();
	auto &input   = drivers::InputDriver::instance();
	auto &storage = drivers::StorageDriver::instance();
	auto &audio   = drivers::AudioDriver::instance();
	auto &motor   = drivers::MotorDriver::instance();

	display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);

	if (!input.init()) {
		showErrorScreen(
			"Failed to initialize the input subsystem.\n"
			"\n"
			"Make sure both decks' encoders are connected properly and the I/O "
			"controller is present. Refer to the log output for more "
			"information."
		);
		return;
	}
	if (!storage.init("/sd")) {
		showErrorScreen(
			"Failed to initialize the SD card.\n"
			"\n"
			"Ensure the card is inserted properly and formatted with a single "
			"FAT16 or FAT32 partition. Refer to the log output for more "
			"information."
		);
		return;
	}

	audio.init(OUTPUT_SAMPLE_RATE, AUDIO_BUFFER_SIZE);
	motor.init();

	auto &audioTask = AudioTask::instance();
	auto &ioTask    = IOTask::instance();
	auto &uiTask    = UITask::instance();

	if (!audioTask.start()) {
		showErrorScreen("Failed to start the audio processing task.");
		return;
	}
	if (!ioTask.start()) {
		showErrorScreen("Failed to start the I/O processing task.");
		return;
	}
	if (!uiTask.start()) {
		showErrorScreen("Failed to start the user interface task.");
		return;
	}

	ESP_LOGI(TAG_, "startup complete");
	heap_caps_print_heap_info(MALLOC_CAP_INVALID);
}

// When building through the Arduino IDE, the ESP32 Arduino core provides its
// own app_main() function which can't be overridden directly.
#ifndef ARDUINO
extern "C" void app_main(void) {
	run();
}
#endif
