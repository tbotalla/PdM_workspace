/**
 * @file API_uart.h
 * @brief Práctica 5 — Punto 1: capa de acceso a la UART (prototipos públicos).
 *
 * Implementación en API_uart.c (HAL, polling, sin IRQ ni DMA según consigna).
 */

#ifndef API_UART_H
#define API_UART_H

#include <stdint.h>

#include "API_delay.h"

/*
 * Contrato previsto (validar en API_uart.c):
 * - Punteros distintos de NULL.
 * - En uartSendStringSize / uartReceiveStringSize: size en [1, 256].
 * - Comprobar retorno de HAL_UART_Transmit / HAL_UART_Receive.
 */

/**
 * @brief Inicializa la UART y envía por terminal un mensaje con sus parámetros.
 * @retval true  inicialización y envío de banner OK.
 * @retval false fallo en HAL o parámetros inválidos.
 */
bool_t uartInit(void);

/**
 * @brief Envía una cadena terminada en '\0' por UART (HAL_UART_Transmit).
 * @param pstring puntero al primer carácter; no debe ser NULL.
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief Envía exactamente @a size bytes desde @a pstring.
 * @param pstring buffer a enviar; no debe ser NULL.
 * @param size cantidad de bytes (1 … 256).
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief Recibe @a size bytes por UART en polling (HAL_UART_Receive).
 * @param pstring buffer destino; no debe ser NULL.
 * @param size cantidad de bytes a leer (1 … 256).
 */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);

#endif /* API_UART_H */
