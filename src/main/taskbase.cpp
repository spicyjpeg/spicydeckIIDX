
#include <assert.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "src/main/defs.hpp"
#include "src/main/taskbase.hpp"

static const char TAG_[]{ "task" };

/* Base class for task singletons */

static constexpr size_t TASK_STACK_SIZE_    = 0x1000;
static constexpr size_t MESSAGE_QUEUE_SIZE_ = 4;

bool Task::start(void) {
	if (task_) {
		ESP_LOGE(TAG_, "%s already started", name_);
		return false;
	}

	queue_   = xQueueCreate(MESSAGE_QUEUE_SIZE_, sizeof(TaskMessage));
	lastRun_ = xTaskGetTickCount();

	if (xTaskCreatePinnedToCore(
		[](void *arg) {
			auto task = reinterpret_cast<Task *>(arg);

			task->mainInit_();

			for (;;) {
				if (xQueueReceive(
					task->queue_,
					&task->lastMessage_,
					0
				) == pdTRUE)
					task->handleMessage_(task->lastMessage_);

				task->mainLoop_();

				if (task->period_)
					xTaskDelayUntil(&task->lastRun_, task->period_);
			}
		},
		name_,
		TASK_STACK_SIZE_,
		this,
		priority_,
		&task_,
		0
	) != pdPASS) {
		task_ = nullptr;

		ESP_LOGE(TAG_, "%s startup failed", name_);
		return false;
	}

	ESP_LOGI(TAG_, "started %s, priority=%d", name_, priority_);
	return true;
}

void Task::stop(void) {
	if (!task_)
		return;

	vQueueDelete(queue_);
	vTaskDelete(task_);
	task_  = nullptr;
	queue_ = nullptr;

	ESP_LOGI(TAG_, "stopped %s", name_);
}

void Task::sendMessage(const TaskMessage &message) {
	if (task_)
		xQueueSendToBack(queue_, &message, portMAX_DELAY);
}
