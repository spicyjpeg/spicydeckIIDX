
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/display.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/defs.hpp"
#include "src/main/taskbase.hpp"
#include "src/main/uitask.hpp"

/* Screen classes */

void MainScreen::draw(UITask &task) const {
	task.font_.draw(
		task.gfx_,
		8,
		8,
		DISPLAY_WIDTH  - 16,
		DISPLAY_HEIGHT - 16,
		"PLACEHOLDER:\nMain screen",
		UI_COLOR_TEXT
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
		UI_COLOR_TEXT
	);
}

void LibraryScreen::update(UITask &task, const drivers::InputState &inputs) {
	// TODO: implement
}

/* Main UI rendering task */

void UITask::mainInit_(void) {
	gfx_.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
	font_.initDefault();

	currentScreen_ = &mainScreen_;
}

void UITask::mainLoop_(void) {
	auto &display = drivers::DisplayDriver::instance();

	gfx_.clear(UI_COLOR_BACKGROUND);
	gfx_.resetClip();
	currentScreen_->draw(*this);

	display.updateAsync(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, gfx_.flip());
}

void UITask::handleMessage_(const TaskMessage &message) {
	switch (message.type) {
		case MESSAGE_INPUTS:
			currentScreen_->update(*this, message.inputs);
			break;

		case MESSAGE_DECK_STATE:
			// TODO: handle deck state
			break;

		default:
			break;
	}
}

UITask &UITask::instance(void) {
	static UITask task;

	return task;
}
