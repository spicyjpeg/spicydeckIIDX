
#pragma once

#include "driver/sdmmc_types.h"

namespace drivers {

/* SD card initialization */

class StorageDriver {
private:
	sdmmc_card_t *card_;
	char         mountPoint_[16];

	inline StorageDriver(void) :
		card_(nullptr)
	{}
	inline ~StorageDriver(void) {
		release();
	}

public:
	static StorageDriver &instance(void);
	bool init(const char *mountPoint);
	void release(void);
};

}
