#include "driver.h"
#include "stm32_f446re_port.h"
#include "stm32f4xx_hal.h"

#define UART_P(h) ((UART_HandleTypeDef *)(void *)(h))
#define I2C_P(h)  ((I2C_HandleTypeDef *)(void *)(h))

uint32_t driver_get_tick_ms() {
    return port_f446re_hal_get_tick();
}

bool_t driver_uart_init(void *huart) {
    if (huart == NULL) {
        return false;
    }
    return port_f446re_hal_uart_init_usart2(UART_P(huart)) == HAL_OK ? true : false;
}

bool_t driver_uart_transmit(void *huart, uint8_t *data, uint16_t len, uint32_t timeout_ms) {
    if (huart == NULL) {
        return false;
    }
    return port_f446re_hal_uart_transmit(UART_P(huart), data, len, timeout_ms) == HAL_OK ? true : false;
}

bool_t driver_uart_receive(void *huart, uint8_t *data, uint16_t len, uint32_t timeout_ms) {
    if (huart == NULL) {
        return false;
    }
    return port_f446re_hal_uart_receive(UART_P(huart), data, len, timeout_ms) == HAL_OK ? true : false;
}

bool_t driver_uart_receive_try(void *huart, uint8_t *data, uint16_t len) {
    if (huart == NULL) {
        return false;
    }
    return port_f446re_hal_uart_receive_nonblock(UART_P(huart), data, len) == HAL_OK ? true : false;
}

bool_t driver_i2c_mem_read(void *hi2c, uint16_t dev_addr, uint16_t reg, uint8_t *data, uint16_t len,
                           uint32_t timeout_ms) {
    if (hi2c == NULL) {
        return false;
    }
    return port_f446re_hal_i2c_mem_read(I2C_P(hi2c), dev_addr, reg, data, len, timeout_ms) == HAL_OK
               ? true
               : false;
}

bool_t driver_i2c_mem_write8(void *hi2c, uint16_t dev_addr, uint16_t reg, uint8_t value,
                             uint32_t timeout_ms) {
    if (hi2c == NULL) {
        return false;
    }
    return port_f446re_hal_i2c_mem_write8(I2C_P(hi2c), dev_addr, reg, value, timeout_ms) == HAL_OK
               ? true
               : false;
}

void driver_buzzer_init() {
    port_f446re_hal_gpio_buzzer_init_idle();
}

void driver_buzzer_on() {
    port_f446re_hal_gpio_buzzer_on();
}

void driver_buzzer_off() {
    port_f446re_hal_gpio_buzzer_off();
}
