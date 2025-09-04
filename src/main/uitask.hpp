
#pragma once

#include "src/main/drivers/input.hpp"
#include "src/main/renderer/font.hpp"
#include "src/main/renderer/renderer.hpp"
#include "src/main/taskbase.hpp"

/* Color palette */

enum UIColor : renderer::Color {
	UI_COLOR_BACKGROUND = renderer::rgb( 16,  16,  6),
	UI_COLOR_TITLE      = renderer::rgb(255, 255, 255),
	UI_COLOR_TEXT       = renderer::rgb(200, 200, 200),
	UI_COLOR_STATUS     = renderer::rgb(160, 160, 160),
	UI_COLOR_ERROR      = renderer::rgb(255,  48,  32)
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
