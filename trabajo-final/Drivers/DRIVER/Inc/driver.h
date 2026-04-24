#ifndef DRIVER_H
#define DRIVER_H

#include "types.h"
#include <stdint.h>

/* Peripheral handles are opaque (void*): the caller passes pointers to the HAL huart / hi2c
 * (e.g. &huart2, &hi2c1). driver.c casts internally. This allows us to switch to a different port without
 * changing this code */
uint32_t driver_get_tick_ms();

bool_t driver_uart_init(void *huart);

bool_t driver_uart_transmit(void *huart, uint8_t *data, uint16_t len, uint32_t timeout_ms);

bool_t driver_uart_receive(void *huart, uint8_t *data, uint16_t len, uint32_t timeout_ms);

bool_t driver_uart_receive_try(void *huart, uint8_t *data, uint16_t len);

bool_t driver_i2c_mem_read(void *hi2c, uint16_t dev_addr, uint16_t reg, uint8_t *data, uint16_t len,
                           uint32_t timeout_ms);

bool_t driver_i2c_mem_write8(void *hi2c, uint16_t dev_addr, uint16_t reg, uint8_t value,
                             uint32_t timeout_ms);

void driver_buzzer_init();

void driver_buzzer_on();

void driver_buzzer_off();

#endif /* DRIVER_H */
