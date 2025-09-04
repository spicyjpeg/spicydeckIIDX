
#pragma once

#include <stdint.h>
#include "driver/spi_master.h"
#include "src/main/util/templates.hpp"

namespace drivers {

/* Display manager class */

class DisplayDriver {
private:
	spi_device_handle_t device_;
	util::Data          asyncTransactions_;

	inline DisplayDriver(void) :
		device_(nullptr)
	{}
	inline ~DisplayDriver(void) {
		release();
	}

public:
	static DisplayDriver &instance(void);
	void init(int width, int height);
	void release(void);

	bool updateAsync(int x, int y, int width, int height, const uint16_t *data);
	void setBacklight(float brightness);
};

}
