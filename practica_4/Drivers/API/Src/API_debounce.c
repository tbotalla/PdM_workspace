#include "API_debounce.h"
#include "stm32f4xx_hal.h"
#include "stdbool.h"

typedef enum {
	BUTTON_UP, BUTTON_FALLING, BUTTON_DOWN, BUTTON_RAISING,
} debounceState_t;

static debounceState_t state = { };
static bool_t button_down;
static const uint32_t DEBOUNCE_TIME = 40;

// getter: expone button_down
bool_t readKey() {
	// devuelve por copia el estado del flag para ver si hubo una
	// pulsacion validada y la resetea
	// el button_down lo setea el buttonPressed()
	return button_down;
}

void debounceFSM_init() {
	state = BUTTON_UP;
}
void debounceFSM_update() {
	bool button_down = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET;
	switch (state) {
	case BUTTON_UP:
		if (button_down) {
			state = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		if (button_down) {
			state = BUTTON_DOWN;
			buttonPressed(); // Emit event
		} else {
			state = BUTTON_UP;
		}
		break;
	case BUTTON_DOWN:
		if (!button_down) {
			state = BUTTON_RAISING;
		}
		break;
	case BUTTON_RAISING:
		if (button_down) {
			state = BUTTON_DOWN;
		} else {
			state = BUTTON_UP;
			buttonReleased(); // Emit event
		}
		break;
	}
}
