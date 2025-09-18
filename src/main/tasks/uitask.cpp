
#include <assert.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/display.hpp"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/inputdefs.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/streamtask.hpp"
#include "src/main/tasks/uitask.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"
#include "src/main/sst.hpp"

namespace tasks {

/* Waveform renderer */

static constexpr int WAVEFORM_HEIGHT_ = dsp::WAVEFORM_RANGE * 2 + 1;

IRAM_ATTR static void drawWaveform_(
	renderer::Renderer &gfx,
	const DeckState    &state,
	const util::Data   &waveform,
	int                y
) {
	gfx.fill(0, y, DISPLAY_WIDTH, WAVEFORM_HEIGHT_, UI_COLOR_WINDOW1);

	if (!waveform.ptr)
		return;

	const float time =
		state.getCurrentTime() * float(dsp::WAVEFORM_SAMPLE_RATE);

	int offset = int(time + 0.5f);
	offset    -= DISPLAY_WIDTH / 2;

	auto      ptr = &waveform.as<uint8_t>()[offset / 2];
	int       x1  = util::max<int>(0, DISPLAY_WIDTH / 2 - offset);
	const int x2  = util::min<int>(DISPLAY_WIDTH, waveform.length * 2 - offset);

	y += dsp::WAVEFORM_RANGE;

	for (; x1 < x2; x1 += 2, offset += 2, ptr++) {
		if (offset < 0)
			continue;

		const int value1 = *ptr & 15;
		const int value2 = *ptr >> 4;

		gfx.verticalLine(x1 + 0, y - value1, value1 * 2 + 1, UI_COLOR_ACCENT1);
		gfx.verticalLine(x1 + 1, y - value2, value2 * 2 + 1, UI_COLOR_ACCENT1);
	}
}

/* Main status screen */

static constexpr int TEXT_MARGIN_     = 8;
static constexpr int WAVEFORM_MARGIN_ = 5;

void MainScreen::draw(UITask &task) const {
	auto &audioTask  = AudioTask::instance();
	auto &streamTask = StreamTask::instance();

	const int lineHeight    = task.font_.getHeader()->lineHeight;
	const int sectionHeight =
		lineHeight * 3 + WAVEFORM_MARGIN_ + WAVEFORM_HEIGHT_;

	int titleY    = (DISPLAY_HEIGHT - sectionHeight    * drivers::NUM_DECKS) / 2;
	int waveformY = (DISPLAY_HEIGHT - WAVEFORM_HEIGHT_ * drivers::NUM_DECKS) / 2;

	for (int i = 0; i < drivers::NUM_DECKS; i++) {
		auto      header    = streamTask.getSSTHeader(i);
		auto      &waveform = streamTask.getSSTWaveform(i);
		DeckState state;

		if (header) {
			audioTask.getDeckState(state, i);

			task.font_.draw(
				task.gfx_,
				TEXT_MARGIN_,
				titleY,
				DISPLAY_WIDTH - TEXT_MARGIN_ * 2,
				lineHeight,
				header->getArtist(),
				UI_COLOR_TEXT1
			);
			titleY += lineHeight;

			task.font_.draw(
				task.gfx_,
				TEXT_MARGIN_,
				titleY,
				DISPLAY_WIDTH - TEXT_MARGIN_ * 2,
				lineHeight,
				header->getTitle(),
				UI_COLOR_TITLE
			);
			titleY += lineHeight;

			char buffer[64], keyName[8];
			int  time = int(state.getCurrentTime());

			streamTask.getKeyName(i, keyName);
			snprintf(
				buffer,
				sizeof(buffer),
				"%d:%02d  %s",
				time / 60,
				time % 60,
				keyName
			);
			task.font_.draw(
				task.gfx_,
				TEXT_MARGIN_,
				titleY,
				DISPLAY_WIDTH - TEXT_MARGIN_ * 2,
				lineHeight,
				buffer,
				UI_COLOR_TEXT2
			);
			titleY += lineHeight;
		}

		drawWaveform_(task.gfx_, state, waveform, waveformY);
		titleY    += (WAVEFORM_MARGIN_ + WAVEFORM_HEIGHT_) * 2;
		waveformY += WAVEFORM_HEIGHT_;
	}
}

void MainScreen::update(UITask &task, const drivers::InputState &inputs) {
	if (inputs.buttonsPressed & drivers::BTN_SELECTOR) {
		task.libraryScreen_.loadDirectory("/sd");
		task.currentScreen_ = &task.libraryScreen_;
	}
}

/* Library browser screen */

static constexpr int LIBRARY_TEXT_MARGIN_ = 4;

void LibraryScreen::draw(UITask &task) const {
	const int lineHeight = task.font_.getHeader()->lineHeight;

	int index = selectedEntry_ - 2;

	for (
		int y = LIBRARY_TEXT_MARGIN_;
		y < (DISPLAY_HEIGHT - LIBRARY_TEXT_MARGIN_);
		y += lineHeight, index++
	) {
		if ((index >= -1) && (index < numEntries_))
			task.font_.draw(
				task.gfx_,
				LIBRARY_TEXT_MARGIN_,
				y,
				DISPLAY_WIDTH  - LIBRARY_TEXT_MARGIN_ * 2,
				DISPLAY_HEIGHT - LIBRARY_TEXT_MARGIN_ * 2,
				(index >= 0)              ? entries_[index]  : "[Cancel]",
				(index == selectedEntry_) ? UI_COLOR_ACCENT1 : UI_COLOR_TEXT1
			);
	}
}

void LibraryScreen::update(UITask &task, const drivers::InputState &inputs) {
	selectedEntry_ += inputs.selector;
	selectedEntry_  = util::clamp(selectedEntry_, -1, numEntries_ - 1);

	if (inputs.buttonsPressed & drivers::BTN_SELECTOR) {
		if (selectedEntry_ >= 0) {
			auto &streamTask = StreamTask::instance();

			snprintf(
				selectedPath_,
				MAX_PATH_LENGTH * 2,
				"%s/%s",
				currentDir_,
				entries_[selectedEntry_]
			);
			streamTask.issueCommand(
				lastUsedDeck_,
				STREAM_CMD_OPEN,
				selectedPath_
			);
			lastUsedDeck_ ^= 1;
		}

		task.currentScreen_ = &task.mainScreen_;
	}
}

void LibraryScreen::loadDirectory(const char *root) {
	strncpy(currentDir_, root, MAX_PATH_LENGTH);
	names_.allocate(0x800);
	assert(names_.ptr);

	numEntries_    = 0;
	selectedEntry_ = -1;
	auto dir       = opendir(root);

	if (!dir)
		return;

	while (numEntries_ < MAX_LIBRARY_ENTRIES) {
		auto entry = readdir(dir);

		if (!entry)
			break;

		// Skip directories and files with extensions other than .sst.
		if (entry->d_type != DT_REG)
			continue;

		const size_t nameLength = strlen(entry->d_name);
		auto         extension  = &entry->d_name[nameLength - 4];

		if ((nameLength <= 4) || memcmp(extension, ".sst", 4))
			continue;

		auto name = names_.add(entry->d_name);

		if (!name)
			break;

		entries_[numEntries_++] = name;
	}

	closedir(dir);
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
