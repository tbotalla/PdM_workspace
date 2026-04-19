#ifndef API_UART_H_
#define API_UART_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "types.h"

/**
 * @brief Initialize UART peripheral, CLI buffer and print welcome banner
 * @param huart pointer to UART handle used by this module
 * @retval true initialization successful
 * @retval false NULL handle or HAL error
 */
bool_t uart_init(UART_HandleTypeDef * huart);

/**
 * @brief Transmit a null-terminated string over UART
 * @param pstring pointer to null-terminated string to send
 * @retval true transmission successful
 * @retval false NULL pointer, empty string or HAL error
 */
bool_t uart_send_string(uint8_t * pstring);

/**
 * @brief Transmit a fixed number of bytes over UART
 * @param pstring pointer to data buffer
 * @param size number of bytes to send (1...256)
 * @retval true transmission successful
 * @retval false invalid parameters or HAL error
 */
bool_t uart_send_string_size(uint8_t *pstring, uint16_t size);

/**
 * @brief Receive a fixed number of bytes over UART
 * @param pstring pointer to destination buffer
 * @param size number of bytes to receive (1...256)
 * @retval true reception successful
 * @retval false invalid parameters or HAL error
 */
bool_t uart_receive_string_size(uint8_t *pstring, uint16_t size);

/**
 * @brief Get total number of HAL UART errors since init
 * @retval error count
 */
uint32_t uart_get_error_count();

/**
 * @brief Try to receive a single byte without blocking
 * @param pstring pointer where received byte is written
 * @retval true byte received
 * @retval false no data available or NULL pointer
 */
bool_t uart_receive_byte_try(uint8_t * pstring);

/**
 * @brief Poll CLI: print periodic status and process incoming commands
 */
void uart_cli_poll();

#endif /* API_UART_H_ */
