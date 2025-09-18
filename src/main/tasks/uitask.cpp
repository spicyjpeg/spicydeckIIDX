
#include <assert.h>
#include <dirent.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/display.hpp"
#include "src/main/drivers/input.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/uitask.hpp"

namespace tasks {

/* Screen classes */

void MainScreen::draw(UITask &task) const {
	task.font_.draw(
		task.gfx_,
		8,
		8,
		DISPLAY_WIDTH  - 16,
		DISPLAY_HEIGHT - 16,
		"PLACEHOLDER:\nMain screen",
		UI_COLOR_TEXT1
	);
}

void MainScreen::update(UITask &task, const drivers::InputState &inputs) {
	// TODO: implement
}

void LibraryScreen::draw(UITask &task) const {
	task.font_.draw(
		task.gfx_,
		8,
		8,
		DISPLAY_WIDTH  - 16,
		DISPLAY_HEIGHT - 16,
		"PLACEHOLDER:\nLibrary screen",
		UI_COLOR_TEXT1
	);
}

void LibraryScreen::update(UITask &task, const drivers::InputState &inputs) {
	// TODO: implement
}

/* Main UI rendering task */

static constexpr int    TASK_PERIOD_        = 20;
static constexpr size_t INPUT_QUEUE_LENGTH_ = 8;

[[noreturn]] void UITask::taskMain_(void) {
	auto &displayDriver = drivers::DisplayDriver::instance();

	gfx_.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	font_.initDefault();

	bool ok = inputQueue_.allocate(INPUT_QUEUE_LENGTH_);
	assert(ok);

	currentScreen_ = &mainScreen_;
	auto lastRun   = xTaskGetTickCount();

	for (;;) {
		drivers::InputState inputs;

		while (inputQueue_.pop(inputs))
			currentScreen_->update(*this, inputs);

		gfx_.clear(UI_COLOR_BACKGROUND);
		gfx_.resetClip();
		currentScreen_->draw(*this);

		displayDriver.updateAsync(
			0,
			0,
			DISPLAY_WIDTH,
			DISPLAY_HEIGHT,
			gfx_.flip()
		);
		xTaskDelayUntil(&lastRun, pdMS_TO_TICKS(TASK_PERIOD_));
	}
}

UITask &UITask::instance(void) {
	static UITask task;

	return task;
}

}
