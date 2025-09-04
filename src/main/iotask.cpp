
#include "src/main/drivers/input.hpp"
#include "src/main/drivers/motor.hpp"
#include "src/main/audiotask.hpp"
#include "src/main/iotask.hpp"
#include "src/main/taskbase.hpp"
#include "src/main/uitask.hpp"

/* Deck object */

static constexpr float DECK_SPEED_RANGE_ = 0.16f;

void DeckIO::init_(void) {
	direction_ = 0;
	targetRPS_ = 0.0f;

	pid_.kp     = 0.1f;
	pid_.ki     = 0.08f;
	pid_.kd     = 0.0005f;
	pid_.iclamp = 1.0f;
}

float DeckIO::updateMeasuredSpeed_(int16_t value, float dt) {
	// Do not update the PID controller while playback is paused.
	if (!direction_)
		return 0.0f;

	float rps = float(value) / dt;
	rps      /= float(drivers::DECK_STEPS_PER_REV);

	return pid_.update(targetRPS_ - rps, dt) * float(direction_);
}

void DeckIO::updateTargetSpeed_(uint8_t value) {
	float rate = float(value) / 127.5f;
	rate       = (rate - 1.0f) * DECK_SPEED_RANGE_;
	rate      += 1.0f;

	targetRPS_ = rate * (DECK_TARGET_RPM / 60.0f);
}

/* Main input polling and motor control task */

void IOTask::mainInit_(void) {
	for (auto &deck : decks_)
		deck.init_();
}

void IOTask::mainLoop_(void) {
	auto &input = drivers::InputDriver::instance();
	auto &motor = drivers::MotorDriver::instance();

	// Poll inputs and send the result to all other tasks.
	TaskMessage message;

	message.type = MESSAGE_INPUTS;
	message.deck = 0;
	input.poll(message.inputs);

	AudioTask::instance().sendMessage(message);
	UITask::instance().sendMessage(message);

	// Update each deck's PID controller.
	decks_[0].updateTargetSpeed_(
		message.inputs.analog[drivers::ANALOG_LEFT_SPEED]
	);
	decks_[1].updateTargetSpeed_(
		message.inputs.analog[drivers::ANALOG_RIGHT_SPEED]
	);

	for (int i = 0; i < drivers::NUM_DECKS; i++) {
		float pidSpeed = decks_[i].updateMeasuredSpeed_(
			message.inputs.decks[i],
			message.inputs.dt
		);

		motor.motors[i].run(pidSpeed);
	}
}

void IOTask::handleMessage_(const TaskMessage &message) {
	switch (message.type) {
		case MESSAGE_DECK_STATE:
			// TODO: handle deck state
			break;

		default:
			break;
	}
}

IOTask &IOTask::instance(void) {
	static IOTask task;

	return task;
}
