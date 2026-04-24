#include "stm32_f446re_port.h"

/* PA8: active-low buzzer, PA5: LD2 (Nucleo) */
#define PORT_BUZZER_PORT GPIOA
#define PORT_BUZZER_PIN  GPIO_PIN_8
#define PORT_LD2_PORT    GPIOA
#define PORT_LD2_PIN     GPIO_PIN_5

uint32_t port_f446re_hal_get_tick() {
    return HAL_GetTick();
}

HAL_StatusTypeDef port_f446re_hal_uart_init_usart2(UART_HandleTypeDef *huart) {
    if (huart == NULL) {
        return HAL_ERROR;
    }
    huart->Instance = USART2;
    huart->Init.BaudRate = 115200;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    return HAL_UART_Init(huart);
}

HAL_StatusTypeDef port_f446re_hal_uart_transmit(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len,
                                                uint32_t timeout_ms) {
    return HAL_UART_Transmit(huart, data, len, timeout_ms);
}

HAL_StatusTypeDef port_f446re_hal_uart_receive(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len,
                                               uint32_t timeout_ms) {
    return HAL_UART_Receive(huart, data, len, timeout_ms);
}

HAL_StatusTypeDef port_f446re_hal_uart_receive_nonblock(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len) {
    return HAL_UART_Receive(huart, data, len, 0U);
}

HAL_StatusTypeDef port_f446re_hal_i2c_mem_read(I2C_HandleTypeDef *hi2c, uint16_t dev_addr, uint16_t reg,
                                               uint8_t *data, uint16_t len, uint32_t timeout_ms) {
    return HAL_I2C_Mem_Read(hi2c, dev_addr, reg, I2C_MEMADD_SIZE_8BIT, data, len, timeout_ms);
}

HAL_StatusTypeDef port_f446re_hal_i2c_mem_write8(I2C_HandleTypeDef *hi2c, uint16_t dev_addr, uint16_t reg,
                                                 uint8_t value, uint32_t timeout_ms) {
    return HAL_I2C_Mem_Write(hi2c, dev_addr, reg, I2C_MEMADD_SIZE_8BIT, &value, 1U, timeout_ms);
}

void port_f446re_hal_gpio_buzzer_init_idle() {
    HAL_GPIO_WritePin(PORT_BUZZER_PORT, PORT_BUZZER_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORT_LD2_PORT, PORT_LD2_PIN, GPIO_PIN_RESET);
}

void port_f446re_hal_gpio_buzzer_on() {
    HAL_GPIO_WritePin(PORT_BUZZER_PORT, PORT_BUZZER_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PORT_LD2_PORT, PORT_LD2_PIN, GPIO_PIN_SET);
}

void port_f446re_hal_gpio_buzzer_off() {
    HAL_GPIO_WritePin(PORT_BUZZER_PORT, PORT_BUZZER_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORT_LD2_PORT, PORT_LD2_PIN, GPIO_PIN_RESET);
}
