
#pragma once

#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_types.h"

namespace drivers {

/* Brushed DC motor object */

class DCMotor {
	friend class MotorDriver;

private:
	mcpwm_oper_handle_t  operator_;
	mcpwm_cmpr_handle_t  comparators_[2];
	mcpwm_gen_handle_t   generators_[2];

	inline DCMotor(void) :
		operator_(nullptr)
	{}
	inline ~DCMotor(void) {
		release_();
	}

	void init_(
		mcpwm_timer_handle_t           timer,
		const mcpwm_generator_config_t *config
	);
	void release_(void);

public:
	void run(float speed);
	void stop(bool brake = false);
};

/* Motor manager class */

class MotorDriver {
private:
	mcpwm_timer_handle_t timer_;

	inline MotorDriver(void) :
		timer_(nullptr)
	{}
	inline ~MotorDriver(void) {
		release();
	}

public:
	DCMotor motors[2];

	static MotorDriver &instance(void);
	void init(void);
	void release(void);
};

}
