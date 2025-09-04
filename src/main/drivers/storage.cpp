
#include <stdint.h>
#include <string.h>
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "src/main/drivers/storage.hpp"
#include "src/main/defs.hpp"

namespace drivers {

static const char TAG_[]{ "storage" };

/* SD card initialization */

static const sdmmc_host_t HOST_CONFIG_ = SDMMC_HOST_DEFAULT();

static const sdmmc_slot_config_t SLOT_CONFIG_{
#if 0
	.clk   = defs::IO_SD_CLK,
	.cmd   = defs::IO_SD_CMD,
	.d0    = defs::IO_SD_DAT,
#else
	// Use dedicated pins to bypass GPIO routing matrix
	.clk   = GPIO_NUM_NC,
	.cmd   = GPIO_NUM_NC,
	.d0    = GPIO_NUM_NC,
#endif
	.d1    = GPIO_NUM_NC,
	.d2    = GPIO_NUM_NC,
	.d3    = GPIO_NUM_NC,
	.d4    = GPIO_NUM_NC,
	.d5    = GPIO_NUM_NC,
	.d6    = GPIO_NUM_NC,
	.d7    = GPIO_NUM_NC,
	.cd    = GPIO_NUM_NC,
	.wp    = GPIO_NUM_NC,
	.width = 1,
	.flags = 0
};

static const esp_vfs_fat_sdmmc_mount_config_t MOUNT_CONFIG_{
	.format_if_mount_failed   = false,
	.max_files                = 8,
	.allocation_unit_size     = 0,
	.disk_status_check_enable = false,
	.use_one_fat              = false
};

StorageDriver &StorageDriver::instance(void) {
	static StorageDriver driver;

	return driver;
}

bool StorageDriver::init(const char *mountPoint) {
	if (card_)
		release();

	const auto error = esp_vfs_fat_sdmmc_mount(
		mountPoint,
		&HOST_CONFIG_,
		&SLOT_CONFIG_,
		&MOUNT_CONFIG_,
		&card_
	);

	if (error == ESP_FAIL) {
		ESP_LOGE(TAG_, "could not mount SD card filesystem");
		return false;
	} else if (error != ESP_OK) {
		ESP_LOGE(TAG_, "could not initialize SD card");
		return false;
	}

	strncpy(mountPoint_, mountPoint, sizeof(mountPoint_));

	const auto capacity =
		uint64_t(card_->csd.capacity) * uint64_t(card_->csd.sector_size);

	ESP_LOGI(TAG_, "SD name:     %s",      card_->cid.name);
	ESP_LOGI(TAG_, "SD capacity: %llu MB", capacity / 0x100000);
	ESP_LOGI(TAG_, "SD speed:    %d MHz",  card_->real_freq_khz / 1000);
	ESP_LOGI(TAG_, "Mount point: %s",      mountPoint);
	return true;
}

void StorageDriver::release(void) {
	if (!card_)
		return;

	esp_vfs_fat_sdcard_unmount(mountPoint_, card_);
	card_ = nullptr;
}

}
