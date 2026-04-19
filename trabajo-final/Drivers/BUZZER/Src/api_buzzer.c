#include "api_buzzer.h"

#include "stm32f4xx_hal.h"

// PA8: active-low buzzer GPIO low = sound ON, high = silent
#define BUZZER_GPIO_PORT GPIOA
#define BUZZER_PIN GPIO_PIN_8
/* LD2 on Nucleo-F446RE: normal LED, high = on */
#define LD2_GPIO_PORT GPIOA
#define LD2_PIN GPIO_PIN_5

void buzzer_init(void) {
    // Buzzer off & LED off
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD2_GPIO_PORT, LD2_PIN, GPIO_PIN_RESET);
}

void buzzer_on(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LD2_GPIO_PORT, LD2_PIN, GPIO_PIN_SET);
}

void buzzer_off(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD2_GPIO_PORT, LD2_PIN, GPIO_PIN_RESET);
}
