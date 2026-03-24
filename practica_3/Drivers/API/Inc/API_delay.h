#ifndef __API_DELAY_H
#define __API_DELAY_H

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t tick_t;
typedef bool bool_t;

typedef struct {
	tick_t startTime;
	tick_t duration;
	bool_t running;
} delay_t;

/**
 * @brief Initialize a non-blocking delay instance.
 *
 * Stores the requested duration and leaves the delay stopped.
 * The time base is provided by `HAL_GetTick()` when `delayRead()` is called.
 */
void delayInit(delay_t *delay, tick_t duration);

/**
 * @brief Non-blocking delay state machine.
 *
 * - If the delay is not running, it captures the current tick and starts it.
 * - If it is running, it checks whether the programmed time has elapsed.
 *
 * When the delay elapses, it returns `true` once and auto-resets `running` to false.
 */
bool_t delayRead(delay_t *delay);

/**
 * @brief Update the duration of an existing delay.
 *
 * Only updates the stored duration value. It does not force a restart; if the
 * delay is already running, subsequent checks will use the new duration.
 * If the duration provided is 0, the duration won't be updated
 */
void delayWrite(delay_t *delay, tick_t duration);

/**
 * @brief Returns if the delay is running
 */
bool_t delayIsRunning(delay_t *delay);

#endif
