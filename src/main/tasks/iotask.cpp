
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/motor.hpp"
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/iotask.hpp"
#include "src/main/tasks/uitask.hpp"

namespace tasks {

/* Deck object */

static constexpr float DECK_SPEED_RANGE_ = 0.16f;

void IOTaskDeck::init_(void) {
	direction_ = 0;
	targetRPS_ = 0.0f;

	pid_.kp     = 0.1f;
	pid_.ki     = 0.08f;
	pid_.kd     = 0.0005f;
	pid_.iclamp = 1.0f;
}

float IOTaskDeck::updateMeasuredSpeed_(int16_t value, float dt) {
	// Do not update the PID controller while playback is paused.
	if (!direction_)
		return 0.0f;

	float rps = float(value) / dt;
	rps      /= float(drivers::DECK_STEPS_PER_REV);

	return pid_.update(targetRPS_ - rps, dt) * float(direction_);
}

void IOTaskDeck::updateTargetSpeed_(uint8_t value) {
	float rate = float(value) / 127.5f;
	rate       = (rate - 1.0f) * DECK_SPEED_RANGE_;
	rate      += 1.0f;

	targetRPS_ = rate * (DECK_TARGET_RPM / 60.0f);
}

/* Main input polling and motor control task */

static constexpr int TASK_PERIOD_ = 10;

[[noreturn]] void IOTask::taskMain_(void) {
	auto &inputDriver = drivers::InputDriver::instance();
	auto &motorDriver = drivers::MotorDriver::instance();
	auto &audioTask   = AudioTask::instance();
	auto &uiTask      = UITask::instance();

	auto lastRun = xTaskGetTickCount();

	for (;;) {
		drivers::InputState inputs;

		inputDriver.poll(inputs);
		audioTask.updateInputs(inputs);
		uiTask.updateInputs(inputs);

		// Update each deck's PID controller.
		// TODO: only run motors when deck is playing
		decks_[0].updateTargetSpeed_(inputs.analog[drivers::ANALOG_LEFT_SPEED]);
		decks_[1].updateTargetSpeed_(inputs.analog[drivers::ANALOG_RIGHT_SPEED]);

		for (int i = 0; i < drivers::NUM_DECKS; i++) {
			float pidSpeed = decks_[i].updateMeasuredSpeed_(
				inputs.decks[i],
				inputs.dt
			);

			motorDriver.motors[i].run(pidSpeed);
		}

		xTaskDelayUntil(&lastRun, pdMS_TO_TICKS(TASK_PERIOD_));
	}
}

IOTask &IOTask::instance(void) {
	static IOTask task;

	return task;
}

}
