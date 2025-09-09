
#pragma once

#include "src/main/drivers/input.hpp"
#include "src/main/renderer/font.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/taskbase.hpp"

/* Color palette */

enum UIColor : renderer::RGB888 {
	UI_COLOR_BACKGROUND = renderer::rgb( 16,  16,  16),
	UI_COLOR_ACCENT1    = renderer::rgb(240, 208,  80),
	UI_COLOR_ACCENT2    = renderer::rgb(160, 136,  48),
	UI_COLOR_WINDOW1    = renderer::rgb( 80,  80,  80),
	UI_COLOR_WINDOW2    = renderer::rgb( 36,  36,  36),
	UI_COLOR_WINDOW3    = renderer::rgb(  8,   8,   8),
	UI_COLOR_HIGHLIGHT1 = renderer::rgb(200, 176,  64),
	UI_COLOR_HIGHLIGHT2 = renderer::rgb(160, 136,  48),
	UI_COLOR_PROGRESS1  = renderer::rgb( 72, 192,  16),
	UI_COLOR_PROGRESS2  = renderer::rgb( 32, 120,   0),
	UI_COLOR_BOX1       = renderer::rgb(  0,   0,   0),
	UI_COLOR_BOX2       = renderer::rgb( 40,  40,  40),
	UI_COLOR_TEXT1      = renderer::rgb(224, 224, 224),
	UI_COLOR_TEXT2      = renderer::rgb(112, 112, 112),
	UI_COLOR_TITLE      = renderer::rgb(255, 255, 255),
	UI_COLOR_SUBTITLE   = renderer::rgb(255, 240, 128)
};

/* Screen classes */

static constexpr int DISPLAY_WIDTH  = 160;
static constexpr int DISPLAY_HEIGHT = 128;

class UITask;

class Screen {
public:
	virtual void draw(UITask &task) const {}
	virtual void update(UITask &task, const drivers::InputState &inputs) {}
};

class MainScreen : public Screen {
public:
	void draw(UITask &task) const;
	void update(UITask &task, const drivers::InputState &inputs);
};

class LibraryScreen : public Screen {
public:
	void draw(UITask &task) const;
	void update(UITask &task, const drivers::InputState &inputs);
};

/* Main UI rendering task */

static constexpr int UI_TASK_PERIOD = 20;

class UITask : public Task {
	friend class MainScreen;
	friend class LibraryScreen;

private:
	renderer::Renderer gfx_;
	renderer::Font     font_;
	Screen             *currentScreen_;

	MainScreen    mainScreen_;
	LibraryScreen libraryScreen_;

	inline UITask(void) :
		Task("UITask", 0, configMAX_PRIORITIES - 10, UI_TASK_PERIOD)
	{}

	void mainInit_(void);
	void mainLoop_(void);
	void handleMessage_(const TaskMessage &message);

public:
	static UITask &instance(void);
};
