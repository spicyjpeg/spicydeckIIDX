
#include <assert.h>
#include "driver/mcpwm_prelude.h"
#include "src/main/drivers/motor.hpp"
#include "src/main/util/templates.hpp"
#include "src/main/defs.hpp"

namespace drivers {

/* Brushed DC motor object */

static constexpr int MCPWM_FREQUENCY_ = 50000;
static constexpr int MCPWM_NUM_TICKS_ = 1 << 9;

static const mcpwm_timer_config_t TIMER_CONFIG_{
	.group_id      = defs::DECK_MCPWM_GROUP,
	.clk_src       = MCPWM_TIMER_CLK_SRC_DEFAULT,
	.resolution_hz = MCPWM_NUM_TICKS_ * MCPWM_FREQUENCY_,
	.count_mode    = MCPWM_TIMER_COUNT_MODE_UP,
	.period_ticks  = MCPWM_NUM_TICKS_,
	.intr_priority = 0,
	.flags         = {
		.update_period_on_empty = true,
		.update_period_on_sync  = false,
		.allow_pd               = false
	}
};

static const mcpwm_operator_config_t OPERATOR_CONFIG_{
	.group_id      = defs::DECK_MCPWM_GROUP,
	.intr_priority = 0,
	.flags         = {
		.update_gen_action_on_tez  = true,
		.update_gen_action_on_tep  = false,
		.update_gen_action_on_sync = false,
		.update_dead_time_on_tez   = true,
		.update_dead_time_on_tep   = false,
		.update_dead_time_on_sync  = false
	}
};

static const mcpwm_comparator_config_t COMPARATOR_CONFIG_{
	.intr_priority = 0,
	.flags         = {
		.update_cmp_on_tez  = true,
		.update_cmp_on_tep  = false,
		.update_cmp_on_sync = false
	}
};

void DCMotor::init_(
	mcpwm_timer_handle_t           timer,
	const mcpwm_generator_config_t *config
) {
	if (operator_)
		release_();

	mcpwm_new_operator(&OPERATOR_CONFIG_, &operator_);
	mcpwm_operator_connect_timer(operator_, timer);

	for (int i = 0; i < 2; i++) {
		mcpwm_new_comparator(operator_, &COMPARATOR_CONFIG_, &comparators_[i]);
		mcpwm_comparator_set_compare_value(comparators_[i], 0);

		mcpwm_new_generator(operator_, &config[i], &generators_[i]);
		mcpwm_generator_set_actions_on_timer_event(
			generators_[i],
			MCPWM_GEN_TIMER_EVENT_ACTION(
				MCPWM_TIMER_DIRECTION_UP,
				MCPWM_TIMER_EVENT_EMPTY,
				MCPWM_GEN_ACTION_HIGH
			),
			MCPWM_GEN_TIMER_EVENT_ACTION_END()
		);
		mcpwm_generator_set_actions_on_compare_event(
			generators_[i],
			MCPWM_GEN_COMPARE_EVENT_ACTION(
				MCPWM_TIMER_DIRECTION_UP,
				comparators_[i],
				MCPWM_GEN_ACTION_LOW
			),
			MCPWM_GEN_COMPARE_EVENT_ACTION_END()
		);
		mcpwm_generator_set_force_level(generators_[i], 0, true);
	}
}

void DCMotor::release_(void) {
	if (!operator_)
		return;

	for (int i = 0; i < 2; i++) {
		mcpwm_del_generator(generators_[i]);
		mcpwm_del_comparator(comparators_[i]);
	}

	mcpwm_del_operator(operator_);
	operator_ = nullptr;
}

void DCMotor::run(float speed) {
	assert(operator_);

	int value = int(speed * float(MCPWM_NUM_TICKS_) + 0.5f);
	int enable1, enable2;

	if (speed >= 0.0f) {
		value   = util::clamp(value, 0, MCPWM_NUM_TICKS_ - 1);
		enable1 = -1;
		enable2 =  0;
	} else {
		value   = util::clamp(-value, 0, MCPWM_NUM_TICKS_ - 1);
		enable1 =  0;
		enable2 = -1;
	}

	mcpwm_comparator_set_compare_value(comparators_[0], value);
	mcpwm_comparator_set_compare_value(comparators_[1], value);
	mcpwm_generator_set_force_level(generators_[0], enable1, true);
	mcpwm_generator_set_force_level(generators_[1], enable2, true);
}

void DCMotor::stop(bool brake) {
	assert(operator_);

	const int level = brake ? 1 : 0;

	mcpwm_generator_set_force_level(generators_[0], level, true);
	mcpwm_generator_set_force_level(generators_[1], level, true);
}

/* Motor manager class */

static const mcpwm_generator_config_t GENERATOR_CONFIG_[][2]{
	{
		// Left deck motor
		{
			.gen_gpio_num = defs::IO_LEFT_DECK_PWM_A,
			.flags        = {
				.invert_pwm   = false,
				.io_loop_back = false,
				.io_od_mode   = false,
				.pull_up      = false,
				.pull_down    = false
			}
		}, {
			.gen_gpio_num = defs::IO_LEFT_DECK_PWM_B,
			.flags        = {
				.invert_pwm   = false,
				.io_loop_back = false,
				.io_od_mode   = false,
				.pull_up      = false,
				.pull_down    = false
			}
		}
	}, {
		// Right deck motor
		{
			.gen_gpio_num = defs::IO_RIGHT_DECK_PWM_A,
			.flags        = {
				.invert_pwm   = false,
				.io_loop_back = false,
				.io_od_mode   = false,
				.pull_up      = false,
				.pull_down    = false
			}
		}, {
			.gen_gpio_num = defs::IO_RIGHT_DECK_PWM_B,
			.flags        = {
				.invert_pwm   = false,
				.io_loop_back = false,
				.io_od_mode   = false,
				.pull_up      = false,
				.pull_down    = false
			}
		}
	}
};

MotorDriver &MotorDriver::instance(void) {
	static MotorDriver driver;

	return driver;
}

void MotorDriver::init(void) {
	mcpwm_new_timer(&TIMER_CONFIG_, &timer_);

	for (int i = 0; i < 2; i++)
		motors[i].init_(timer_, GENERATOR_CONFIG_[i]);

	mcpwm_timer_enable(timer_);
	mcpwm_timer_start_stop(timer_, MCPWM_TIMER_START_NO_STOP);
}

void MotorDriver::release(void) {
	if (!timer_)
		return;

	mcpwm_timer_start_stop(timer_, MCPWM_TIMER_STOP_EMPTY);
	mcpwm_timer_disable(timer_);

	for (auto &motor : motors)
		motor.release_();

	mcpwm_del_timer(timer_);
	timer_ = nullptr;
}

}
