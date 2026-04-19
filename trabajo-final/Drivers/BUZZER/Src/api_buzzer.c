#include "api_buzzer.h"

#include "stm32f4xx_hal.h"

// Buzzer configured on pin PA8
#define BUZZER_GPIO_PORT GPIOA
#define BUZZER_PIN GPIO_PIN_8

void buzzer_init(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void buzzer_on(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void buzzer_off(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}
