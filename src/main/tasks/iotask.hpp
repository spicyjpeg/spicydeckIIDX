
#pragma once

#include <stdint.h>
#include "src/main/drivers/input.hpp"
#include "src/main/dsp/dsp.hpp"
#include "src/main/util/rtos.hpp"

namespace tasks {

static constexpr float DECK_TARGET_RPM = 45.0;

/* Deck object */

class IOTaskDeck {
	friend class IOTask;

private:
	dsp::PIDController pid_;
	float              targetRPS_;

	void init_(void);
	float updateMeasuredSpeed_(int16_t value, float dt);
	void updateTargetSpeed_(uint8_t value, bool reverse = false);
};

/* Main input polling and motor control task */

class IOTask : public util::Task {
private:
	IOTaskDeck decks_[drivers::NUM_DECKS];

	inline IOTask(void) :
		Task("IOTask", 0x400)
	{}

	[[noreturn]] void taskMain_(void) override;

public:
	static IOTask &instance(void);
};

}
