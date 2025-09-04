
#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/input.hpp"

/* Base class for task singletons */

enum TaskMessageType : uint8_t {
	MESSAGE_INPUTS     = 0, // IOTask    -> AudioTask, UITask
	MESSAGE_DECK_STATE = 1, // AudioTask -> UITask
	MESSAGE_LOAD_START = 2, // UITask    -> AudioTask
	MESSAGE_LOAD_END   = 3  // UITask    -> AudioTask
};

struct DeckStateMessage {
public:
	// TODO: define
};

struct TaskMessage {
public:
	TaskMessageType type;
	uint8_t         deck;

	union {
		drivers::InputState inputs;
		DeckStateMessage    deckState;
		const char          *path;
	};
};

class Task {
private:
	const char *name_;
	uint16_t   affinity_, priority_;
	TickType_t period_;

	TaskHandle_t  task_;
	QueueHandle_t queue_;

	TickType_t  lastRun_;
	TaskMessage lastMessage_;

	virtual void mainInit_(void) {}
	virtual void mainLoop_(void) {}
	virtual void handleMessage_(const TaskMessage &message) {}

protected:
	inline Task(
		const char *name,
		int        affinity,
		int        priority,
		int        period = 0
	) :
		name_(name),
		affinity_(affinity),
		priority_(priority),
		period_(period / portTICK_PERIOD_MS),
		task_(nullptr),
		queue_(nullptr)
	{}
	inline ~Task(void) {
		stop();
	}

public:
	bool start(void);
	void stop(void);
	void sendMessage(const TaskMessage &message);
};
