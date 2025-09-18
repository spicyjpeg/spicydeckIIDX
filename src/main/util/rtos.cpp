
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "src/main/util/rtos.hpp"

namespace util {

/* FreeRTOS wrappers */

[[noreturn]] void Task::taskMain_(void) {
	for (;;)
		vTaskDelay(portMAX_DELAY);
}

Task::Task(const char *name, size_t stackLength) :
	handle_(nullptr),
	name_(name),
	stackLength_(stackLength)
{
	assert(name && (stackLength > 0));
}

bool Task::run(int affinity, int priority) {
	if (handle_)
		return true;
	if (!stack_.allocate(stackLength_))
		return false;

	handle_ = xTaskCreateStaticPinnedToCore(
		[](void *arg) {
			reinterpret_cast<Task *>(arg)->taskMain_();
		},
		name_,
		stackLength_,
		this,
		priority,
		stack_.as<StackType_t>(),
		&buffer_,
		affinity
	);

	return !!handle_;
}

void Task::stop(void) {
	if (!handle_)
		return;

	vTaskDelete(handle_);
	stack_.destroy();
	handle_ = nullptr;
}

bool MessageQueue::allocate(size_t length) {
	if (handle_)
		destroy();
	if (!items_.allocate(length))
		return false;

	handle_ = xMessageBufferCreateStatic(
		length,
		items_.as<uint8_t>(),
		&buffer_
	);

	return !!handle_;
}

void MessageQueue::destroy(void) {
	if (!handle_)
		return;

	vMessageBufferDelete(handle_);
	items_.destroy();
	handle_ = nullptr;
}

BinarySemaphore::BinarySemaphore(void) {
	handle_ = xSemaphoreCreateBinaryStatic(&buffer_);

	assert(handle_);
	xSemaphoreGive(handle_);
}

Mutex::Mutex(void) {
	handle_ = xSemaphoreCreateMutexStatic(&buffer_);

	assert(handle_);
	xSemaphoreGive(handle_);
}

}
