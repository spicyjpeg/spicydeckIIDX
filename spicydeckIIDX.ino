
#include "freertos/FreeRTOS.h"

// run() is defined in src/main/main.cpp. It will return after initializing all
// drivers and launching the main tasks, at which point the task provided by the
// ESP32 Arduino core that calls setup() and loop() may be safely terminated.
void run(void);

void setup(void) {
	run();
	vTaskDelete(nullptr);
}

void loop(void) {}
