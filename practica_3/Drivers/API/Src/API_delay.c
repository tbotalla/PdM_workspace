#include "stm32f4xx_hal.h"
#include "API_delay.h"

/**
 * @brief Initialize a non-blocking delay instance.
 *
 * Stores the requested duration and leaves the delay stopped.
 * The time base is provided by `HAL_GetTick()` when `delayRead()` is called.
 */
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

/**
 * @brief Non-blocking delay state machine.
 *
 * - If the delay is not running, it captures the current tick and starts it.
 * - If it is running, it checks whether the programmed time has elapsed.
 *
 * When the delay elapses, it returns `true` once and auto-resets `running` to false.
 */
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

/**
 * @brief Update the duration of an existing delay.
 *
 * Only updates the stored duration value. It does not force a restart; if the
 * delay is already running, subsequent checks will use the new duration.
 * If the duration provided is 0, the duration won't be updated
 */
void delayWrite(delay_t *delay, tick_t duration) {
	if (delay == NULL) {
		return;
	}
	if (delay == 0) {
		return;
	}
	delay->duration = duration;
}

/**
 * @brief Returns if the delay is running
 */
bool_t delayIsRunning(delay_t *delay) {
	if (delay == NULL) {
		return false;
	}
	return delay->running;
}
