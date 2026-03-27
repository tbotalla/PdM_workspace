#include "stm32f4xx_hal.h"
#include "API_delay.h"


void delayInit(delay_t *delay, tick_t duration) {
	if (delay == NULL) {
		return;
	}
	if (duration == 0) {
		return;
	}
	delay->duration = duration;
	delay->running = false;
}

bool_t delayRead(delay_t *delay) {
	if (delay == NULL) {
		return false;
	}
	if (!delay->running) {
		delay->startTime = HAL_GetTick();
		delay->running = true;
		return false;
	}
	if (HAL_GetTick() - delay->startTime >= delay->duration) {
		delay->running = false; // auto-reset once the time is completed
		return true;
	}
	return false;
}

void delayWrite(delay_t *delay, tick_t duration) {
	if (delay == NULL) {
		return;
	}
	if (delay == 0) {
		return;
	}
	delay->duration = duration;
}

bool_t delayIsRunning(delay_t *delay) {
	if (delay == NULL) {
		return false;
	}
	return delay->running;
}
