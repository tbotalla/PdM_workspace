#include "API_uart.h"

#include <string.h>

#include "main.h"
#include "stm32f4xx_hal.h"

UART_HandleTypeDef huart2;

#define UART_TIMEOUT_MS 1000U
#define UART_STR_MAX_LEN 256U
#define UART_SIZE_MIN 1U
#define UART_SIZE_MAX 256U
#define STR_END_CHAR '\0'

void uartSendString(uint8_t *pstring) {
	if (pstring == NULL) {
		return;
	}
	uint8_t i = 0U;
	while (pstring[i] != '\0' && i < UART_STR_MAX_LEN) {
		i++;
	}
	if (i == 0U) {
		return;
	}
	if (HAL_UART_Transmit(&huart2, pstring, i, UART_TIMEOUT_MS) != HAL_OK) {
		// TODO: propagate error to the caller
	}
}

void uartSendStringSize(uint8_t *pstring, uint16_t size) {
	if (pstring == NULL) {
		return;
	}
	if (size < UART_SIZE_MIN || size > UART_SIZE_MAX) {
		return;
	}
	if (HAL_UART_Transmit(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK) {
		// TODO: propagate error to the caller
	}
}

void uartReceiveStringSize(uint8_t *pstring, uint16_t size) {
	if (pstring == NULL) {
		return;
	}
	if (size < UART_SIZE_MIN || size > UART_SIZE_MAX) {
		return;
	}
	if (HAL_UART_Receive(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK) {
		// TODO: Propagate error to the caller
	}
}

bool_t uartInit(void) {
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&huart2) != HAL_OK) {
		return false;
	}

	uartSendString((uint8_t*) "\r\n=== API_uart init ===\r\n");
	uartSendString((uint8_t*) "huart2.Init.BaudRate = 115200\r\n");
	uartSendString(
			(uint8_t*) "huart2.Init.WordLength = UART_WORDLENGTH_8B\r\n");
	uartSendString((uint8_t*) "huart2.Init.StopBits = UART_STOPBITS_1\r\n");
	uartSendString((uint8_t*) "huart2.Init.Parity = UART_PARITY_NONE\r\n");
	uartSendString((uint8_t*) "huart2.Init.Mode = UART_MODE_TX_RX\r\n");
	uartSendString(
			(uint8_t*) "huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE\r\n");
	uartSendString(
			(uint8_t*) "huart2.Init.OverSampling = UART_OVERSAMPLING_16\r\n");
	uartSendString((uint8_t*) "====================\r\n");

	return true;
}
