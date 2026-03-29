#include "stm32f4xx_hal.h"
#include "stdbool.h"

#include "API_debounce.h"

#define DEBOUNCE_TIME_MS 40

typedef enum {
	BUTTON_UP, BUTTON_FALLING, BUTTON_DOWN, BUTTON_RAISING,
} debounceState_t;

static debounceState_t state = { };
static bool_t button_down;
static delay_t delay = { 0 };

static void button_released() {
	button_down = false;
	state = BUTTON_UP;
}

static void button_pressed() {
	button_down = true;
	state = BUTTON_DOWN;
}

bool_t readKey() {
	if (button_down) {
		button_down = false;
		return true;
	}

	return button_down;
}

void debounceFSM_init() {
	button_down = false;
	state = BUTTON_UP;
	delayInit(&delay, DEBOUNCE_TIME_MS);
}
void debounceFSM_update() {
	bool pressed = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET;
	switch (state) {
	case BUTTON_UP:
		if (pressed) {
			state = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		if (delayRead(&delay)) {
			// Debounce time elapsed, stable read
			if (pressed) {
				button_pressed();
			} else {
				state = BUTTON_UP;
			}
		}
		break;
	case BUTTON_DOWN:
		if (!pressed) {
			state = BUTTON_RAISING;
		}
		break;
	case BUTTON_RAISING:
		if (delayRead(&delay)) {
			// Debounce time elapsed, stable read
			if (pressed) {
				state = BUTTON_DOWN;
			} else {
				button_released();
			}
		}
		break;
	default:
		// TODO: error handling
		break;
	}
}
