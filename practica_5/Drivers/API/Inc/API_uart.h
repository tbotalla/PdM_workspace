#ifndef API_UART_H
#define API_UART_H

#include <stdint.h>

#include "API_delay.h"

/**
 * @brief Initializes UART and prints a startup banner.
 * @retval true success
 * @retval false HAL/config error
 */
bool_t uartInit(void);

/**
 * @brief Sends a null-terminated string.
 * @param pstring source pointer (non-NULL)
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief Sends exactly size bytes.
 * @param pstring source buffer (non-NULL)
 * @param size bytes to send (1 to 256)
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief Receives exactly size bytes in polling mode.
 * @param pstring destination buffer (non-NULL)
 * @param size bytes to receive (1 to 256)
 */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief Tries to receive 1 byte (non-blocking). Implemented to avoid modifying the siganture of uartReceiveStringSize
 * @param pstring destination byte pointer (non-NULL)
 * @retval true byte received
 * @retval false no data or error
 */
bool_t uartReceiveByteTry(uint8_t *pstring);

#endif
