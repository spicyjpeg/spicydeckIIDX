
#pragma once

#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/input.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/taskbase.hpp"

static constexpr float DECK_TARGET_RPM = 45.0;

/* Deck object */

class DeckIO {
	friend class IOTask;

private:
	dsp::PIDController pid_;

	int   direction_;
	float targetRPS_;

	inline DeckIO(void) {}

	void init_(void);
	float updateMeasuredSpeed_(int16_t value, float dt);
	void updateTargetSpeed_(uint8_t value);
};

/* Main input polling and motor control task */

static constexpr int IO_TASK_PERIOD = 10;

class IOTask : public Task {
private:
	DeckIO decks_[drivers::NUM_DECKS];

	inline IOTask(void) :
		Task("IOTask", 0, configMAX_PRIORITIES - 2, IO_TASK_PERIOD)
	{}

	void mainInit_(void);
	void mainLoop_(void);
	void handleMessage_(const TaskMessage &message);

public:
	static IOTask &instance(void);
};
