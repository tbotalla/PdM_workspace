#ifndef STM32_F446RE_PORT_H
#define STM32_F446RE_PORT_H

#include "stm32f4xx_hal.h"

uint32_t port_f446re_hal_get_tick();

HAL_StatusTypeDef port_f446re_hal_uart_init_usart2(UART_HandleTypeDef * huart);

HAL_StatusTypeDef port_f446re_hal_uart_transmit(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len,
                                                uint32_t timeout_ms);

HAL_StatusTypeDef port_f446re_hal_uart_receive(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len,
                                               uint32_t timeout_ms);

HAL_StatusTypeDef port_f446re_hal_uart_receive_nonblock(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len);

HAL_StatusTypeDef port_f446re_hal_i2c_mem_read(I2C_HandleTypeDef *hi2c, uint16_t dev_addr, uint16_t reg,
                                               uint8_t *data, uint16_t len, uint32_t timeout_ms);

HAL_StatusTypeDef port_f446re_hal_i2c_mem_write8(I2C_HandleTypeDef *hi2c, uint16_t dev_addr, uint16_t reg,
                                                 uint8_t value, uint32_t timeout_ms);

void port_f446re_hal_gpio_buzzer_init_idle();

void port_f446re_hal_gpio_buzzer_on();

void port_f446re_hal_gpio_buzzer_off();

#endif /* STM32_F446RE_PORT_H */
