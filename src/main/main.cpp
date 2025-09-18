
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/audio.hpp"
#include "src/main/drivers/display.hpp"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/motor.hpp"
#include "src/main/drivers/storage.hpp"
#include "src/main/renderer/font.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/iotask.hpp"
#include "src/main/tasks/streamtask.hpp"
#include "src/main/tasks/uitask.hpp"
#include "src/main/defs.hpp"

static const char TAG_[]{ "main" };

/* Entry point */

static void showErrorScreen(const char *text) {
	renderer::Renderer gfx;
	renderer::Font     font;

	gfx.init(tasks::DISPLAY_WIDTH, tasks::DISPLAY_HEIGHT);
	gfx.clear(tasks::UI_COLOR_BACKGROUND);

	font.initDefault();
	font.draw(
		gfx,
		8,
		8,
		tasks::DISPLAY_WIDTH - (8 * 2),
		16,
		"Error",
		tasks::UI_COLOR_TITLE
	);
	font.draw(
		gfx,
		8,
		8 + 16,
		tasks::DISPLAY_WIDTH  - (8 * 2),
		tasks::DISPLAY_HEIGHT - (8 * 2 + 16),
		text,
		tasks::UI_COLOR_TEXT1,
		true
	);

	auto &displayDriver = drivers::DisplayDriver::instance();

	displayDriver.updateAsync(
		0,
		0,
		tasks::DISPLAY_WIDTH,
		tasks::DISPLAY_HEIGHT,
		gfx.flip()
	);
}

void run(void) {
	auto &displayDriver = drivers::DisplayDriver::instance();
	auto &inputDriver   = drivers::InputDriver::instance();
	auto &storageDriver = drivers::StorageDriver::instance();
	auto &audioDriver   = drivers::AudioDriver::instance();
	auto &motorDriver   = drivers::MotorDriver::instance();

	displayDriver.init(tasks::DISPLAY_WIDTH, tasks::DISPLAY_HEIGHT);

	if (!inputDriver.init()) {
		showErrorScreen(
			"Failed to initialize the input subsystem.\n"
			"\n"
			"Make sure both decks' encoders are connected properly and the I/O "
			"controller is present. Refer to the log output for more "
			"information."
		);
		return;
	}
	if (!storageDriver.init("/sd")) {
		showErrorScreen(
			"Failed to initialize the SD card.\n"
			"\n"
			"Ensure the card is inserted properly and formatted with a single "
			"FAT16 or FAT32 partition. Refer to the log output for more "
			"information."
		);
		return;
	}

	audioDriver.init(tasks::OUTPUT_SAMPLE_RATE, tasks::AUDIO_BUFFER_SIZE);
	motorDriver.init();

	ESP_LOGI(TAG_, "initialization complete");

	auto &audioTask  = tasks::AudioTask::instance();
	auto &ioTask     = tasks::IOTask::instance();
	auto &streamTask = tasks::StreamTask::instance();
	auto &uiTask     = tasks::UITask::instance();

	if (!audioTask.run(1, configMAX_PRIORITIES - 2)) {
		showErrorScreen("Failed to start the audio processing task.");
		return;
	}
	if (!ioTask.run(1, configMAX_PRIORITIES - 1)) {
		showErrorScreen("Failed to start the I/O processing task.");
		return;
	}
	if (!streamTask.run(0, configMAX_PRIORITIES - 1)) {
		showErrorScreen("Failed to start the audio file streaming task.");
		return;
	}
	if (!uiTask.run(0, configMAX_PRIORITIES / 2)) {
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
