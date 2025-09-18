
#pragma once

#include <assert.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "portmacro.h"
#include "src/main/util/templates.hpp"

namespace util {

/* FreeRTOS wrappers */

class Task {
private:
	StaticTask_t buffer_;
	TaskHandle_t handle_;

	const char *name_;
	size_t     stackLength_;
	Data       stack_;

protected:
	[[noreturn]] virtual void taskMain_(void);

public:
	inline ~Task(void) {
		stop();
	}
	inline void suspend(void) {
		vTaskSuspend(handle_);
	}
	inline void resume(void) {
		vTaskResume(handle_);
	}
	inline void resumeFromISR(void) {
		xTaskResumeFromISR(handle_);
	}

	Task(const char *name, size_t stackLength);
	bool run(
		int affinity = tskNO_AFFINITY,
		int priority = configMAX_PRIORITIES / 2
	);
	void stop(void);
};

template<typename T> class Queue {
private:
	StaticQueue_t buffer_;
	QueueHandle_t handle_;

	Data items_;

public:
	inline ~Queue(void) {
		destroy();
	}
	inline bool allocate(size_t length) {
		if (handle_)
			destroy();
		if (!items_.allocate<T>(length))
			return false;

		handle_ = xQueueCreateStatic(
			length,
			sizeof(T),
			items_.as<uint8_t>(),
			&buffer_
		);

		return !!handle_;
	}
	inline void destroy(void) {
		if (!handle_)
			return;

		vQueueDelete(handle_);
		items_.destroy();
		handle_ = nullptr;
	}

	inline bool push(const T &item, bool blocking = false) {
		return xQueueSendToBack(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline bool pushFirst(const T &item, bool blocking = false) {
		return xQueueSendToFront(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline bool pop(T &item, bool blocking = false) {
		return xQueueReceive(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline bool peek(T &item, bool blocking = false) {
		return xQueuePeek(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline void clear(void) {
		xQueueReset(handle_);
	}
};

template<typename T> class Mailbox {
private:
	StaticQueue_t buffer_;
	QueueHandle_t handle_;

	T item_;

public:
	inline Mailbox(void) {
		handle_ = xQueueCreateStatic(1, sizeof(T), &item_, &buffer_);

		assert(handle_);
	}
	inline ~Mailbox(void) {
		vQueueDelete(handle_);
	}

	inline bool put(const T &item, bool blocking = false) {
		return xQueueSendToBack(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline bool get(T &item, bool blocking = false) {
		return xQueueReceive(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline bool peek(T &item, bool blocking = false) {
		return xQueuePeek(
			handle_,
			&item,
			blocking ? portMAX_DELAY : 0
		) == pdPASS;
	}
	inline void clear(void) {
		xQueueReset(handle_);
	}
};

template<typename T> class InPlaceQueue {
private:
	StaticRingbuffer_t buffer_;
	RingbufHandle_t    handle_;

	void *pushedItem_, *poppedItem_;
	Data items_;

public:
	inline ~InPlaceQueue(void) {
		destroy();
	}
	inline bool allocate(size_t length) {
		if (handle_)
			destroy();
		if (!items_.allocate<T>(length))
			return false;

		pushedItem_ = nullptr;
		poppedItem_ = nullptr;
		handle_     = xRingbufferCreateStatic(
			items_.length,
			RINGBUF_TYPE_BYTEBUF,
			items_.as<uint8_t>(),
			&buffer_
		);

		return !!handle_;
	}
	inline void destroy(void) {
		if (!handle_)
			return;

		assert(!poppedItem_);

		vRingbufferDelete(handle_);
		items_.destroy();
		handle_ = nullptr;
	}

	inline T *pushItem(bool blocking = false) {
		assert(!pushedItem_);

		xRingbufferSendAcquire(
			handle_,
			&pushedItem_,
			sizeof(T),
			blocking ? 0 : portMAX_DELAY
		);

		return reinterpret_cast<T *>(pushedItem_);
	}
	inline void finalizePush(void) {
		assert(pushedItem_);

		xRingbufferSendComplete(handle_, pushedItem_);
		pushedItem_ = nullptr;
	}
	inline const T *popItem(bool blocking = false) {
		assert(!poppedItem_);

		size_t pending;

		vRingbufferGetInfo(
			handle_,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&pending
		);

		if (pending < sizeof(T))
			return nullptr;

		poppedItem_ = xRingbufferReceiveUpTo(
			handle_,
			&pending,
			blocking ? portMAX_DELAY : 0,
			sizeof(T)
		);

		return reinterpret_cast<const T *>(poppedItem_);
	}
	inline void finalizePop(void) {
		assert(poppedItem_);

		vRingbufferReturnItem(handle_, poppedItem_);
		poppedItem_ = nullptr;
	}
};

class MessageQueue {
private:
	StaticMessageBuffer_t buffer_;
	MessageBufferHandle_t handle_;

	Data items_;

public:
	inline ~MessageQueue(void) {
		destroy();
	}
	inline bool push(const void *item, size_t length, bool blocking = false) {
		return !!xMessageBufferSend(
			handle_,
			item,
			length,
			blocking ? portMAX_DELAY : 0
		);
	}
	template<typename T> inline bool push(const T &item, bool blocking = false) {
		return !!xMessageBufferSend(
			handle_,
			&item,
			sizeof(T),
			blocking ? portMAX_DELAY : 0
		);
	}
	inline size_t pop(void *item, size_t maxLength, bool blocking = false) {
		return xMessageBufferReceive(
			handle_,
			item,
			maxLength,
			blocking ? portMAX_DELAY : 0
		);
	}
	inline void clear(void) {
		xMessageBufferReset(handle_);
	}

	bool allocate(size_t length);
	void destroy(void);
};

class SemaphoreBase {
protected:
	StaticSemaphore_t buffer_;
	SemaphoreHandle_t handle_;

public:
	inline ~SemaphoreBase(void) {
		vSemaphoreDelete(handle_);
	}
	inline bool lock(bool blocking = false) {
		return xSemaphoreTake(handle_, blocking ? portMAX_DELAY : 0) == pdTRUE;
	}
	inline void unlock(void) {
		xSemaphoreGive(handle_);
	}
	inline void unlockFromISR(void) {
		int dummy;

		xSemaphoreGiveFromISR(handle_, &dummy);
	}
};

class BinarySemaphore : public SemaphoreBase {
public:
	BinarySemaphore(void);
};

class Mutex : public SemaphoreBase {
public:
	Mutex(void);
};

}
