#include "api_uart.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "stm32f4xx_hal.h"
#include "api_thermal.h"

#define UART_TIMEOUT_MS 1000U // Max blocking time (in ms) for HAL UART transmit/receive
#define UART_STR_MAX_LEN 256U // Max length scanned by uart_send_string before giving up
#define UART_SIZE_MIN 1U // Minimum accepted size for sized send/receive functions
#define UART_SIZE_MAX 256U // Maximum accepted size for sized send/receive functions
#define STR_END_CHAR '\0' // End of string null terminator
#define CLI_LINE_MAX 96U // Max bytes in the CLI input line buffer (including '\0')
#define CLI_REPORT_PERIOD_MS 3000U // Interval between automatic status reports over UART
#define CLI_DELIMITERS " \t" // Token delimiters for command parsing (space and tab)

// Buffer that accumulates incoming bytes until a full line is received
static uint8_t cli_line[CLI_LINE_MAX];
// Current write position inside cli_line
static uint16_t cli_index = 0U;
// Timestamp of the last periodic status report sent over UART
static uint32_t cli_last_report_ms = 0U;
// Pointer to the UART peripheral handle provided by the caller at init
static UART_HandleTypeDef *uart_huart = NULL;
// Counts HAL UART transmit/receive errors since init
static uint32_t uart_error_count = 0U;


static void split_int_dec_parts(float_t value, long_t *int_part, long_t *dec_part) {
    if ((int_part == NULL) || (dec_part == NULL)) {
        return;
    }

    long_t x100 = (long_t)(value * 100.0f);
    *int_part = x100 / 100;
    *dec_part = (x100 >= 0) ? (x100 % 100) : -(x100 % 100);
}

/* Prints current status of the whole system */
static void uart_send_cmd_status() {
    uint32_t tmax_ms = thermal_fsm_get_max_alarm_ms();
    const char_t *state = thermal_state_to_string(thermal_fsm_get_state());
    long_t temp_int = 0;
    long_t temp_dec = 0;
    long_t threshold_int = 0;
    long_t threshold_dec = 0;
    long_t hysteresis_int = 0;
    long_t hysteresis_dec = 0;
    split_int_dec_parts(thermal_fsm_get_last_temp(), &temp_int, &temp_dec);
    split_int_dec_parts(thermal_fsm_get_threshold(), &threshold_int, &threshold_dec);
    split_int_dec_parts(thermal_fsm_get_hysteresis(), &hysteresis_int,
                        &hysteresis_dec);

    char_t msg[180];
    snprintf(msg, sizeof(msg),
             "STATUS temp=%ld.%02ldC tmp_max=%ld.%02ldC hyst=%ld.%02ldC alarm_timeout=%lums state=%s uart_errors=%lu\r\n",
             temp_int, temp_dec, threshold_int, threshold_dec, hysteresis_int,
             hysteresis_dec, (ulong_t) tmax_ms, state, (ulong_t) uart_get_error_count());
    uart_send_string((uint8_t *) msg);
}

/* Prints the help through UART */
static void uart_send_cmd_help() {
    uart_send_string((uint8_t *) "Commands:\r\n");
    uart_send_string((uint8_t *) "  HELP\r\n");
    uart_send_string((uint8_t *) "  STATUS\r\n");
    uart_send_string((uint8_t *) "  SET TMP_MAX <value_c>\r\n");
    uart_send_string((uint8_t *) "  SET HYST <value_c>\r\n");
    uart_send_string((uint8_t *) "  SET ALARM_TIMEOUT <value_ms>\r\n");
    uart_send_string((uint8_t *) "  GET TMP_MAX|HYST|ALARM_TIMEOUT|STATE|TEMP\r\n");
}

/* Prints current temperature report */
static void uart_send_cmd_temp_report(void) {
    long_t temp_int = 0;
    long_t temp_dec = 0;
    char_t report[96];
    split_int_dec_parts(thermal_fsm_get_last_temp(), &temp_int, &temp_dec);
    snprintf(report, sizeof(report), "TEMP=%ld.%02ldC STATE=%s\r\n",
             temp_int, temp_dec, thermal_state_to_string(thermal_fsm_get_state()));
    uart_send_string((uint8_t *) report);
}

/* Fills the line with 0's */
static void uart_cli_init() {
    cli_index = 0U;
    for (uint16_t i = 0U; i < CLI_LINE_MAX; i++) {
        cli_line[i] = 0U;
    }
    cli_last_report_ms = HAL_GetTick();
}

static void process_cli_command(char_t *line) {
    if (line == NULL) {
        return;
    }

    char_t *cmd = NULL;
    cmd = strtok(line, CLI_DELIMITERS); // Splits the command based on spaces or tabs
    if (cmd == NULL) {
        return;
    }

    char_t *arg1 = NULL;
    char_t *arg2 = NULL;
    arg1 = strtok(NULL, CLI_DELIMITERS); // Gets the first arg
    arg2 = strtok(NULL, CLI_DELIMITERS); // Gets the second arg

    if (strcmp(cmd, "HELP") == 0) {
        uart_send_cmd_help();
        return;
    }

    if (strcmp(cmd, "STATUS") == 0) {
        uart_send_cmd_status();
        return;
    }

    char_t msg[96];
    if (strcmp(cmd, "SET") == 0) {
        if ((arg1 == NULL) || (arg2 == NULL)) {
            uart_send_string((uint8_t *) "ERROR: bad arguments\r\n");
            return;
        }

        if (strcmp(arg1, "TMP_MAX") == 0) {
            long_t threshold_int = 0;
            long_t threshold_dec = 0;
            thermal_fsm_set_threshold((float_t) strtod(arg2, NULL)); // str to double to float_t
            split_int_dec_parts(thermal_fsm_get_threshold(), &threshold_int,
                                &threshold_dec);
            snprintf(msg, sizeof(msg), "OK TMP_MAX=%ld.%02ld\r\n",
                     threshold_int, threshold_dec);
            uart_send_string((uint8_t *) msg);
            return;
        }
        if (strcmp(arg1, "HYST") == 0) {
            long_t hysteresis_int = 0;
            long_t hysteresis_dec = 0;
            thermal_fsm_set_hysteresis((float_t) strtod(arg2, NULL)); // str to double to float_t
            split_int_dec_parts(thermal_fsm_get_hysteresis(), &hysteresis_int,
                                &hysteresis_dec);
            snprintf(msg, sizeof(msg), "OK HYST=%ld.%02ld\r\n",
                     hysteresis_int, hysteresis_dec);
            uart_send_string((uint8_t *) msg);
            return;
        }
        if (strcmp(arg1, "ALARM_TIMEOUT") == 0) {
            thermal_fsm_set_max_alarm_ms((uint32_t) strtoul(arg2, NULL, 10)); // str to unsigned long to uint32_t
            snprintf(msg, sizeof(msg), "OK ALARM_TIMEOUT=%lu\r\n",
                     (ulong_t) thermal_fsm_get_max_alarm_ms());
            uart_send_string((uint8_t *) msg);
            return;
        }

        uart_send_string((uint8_t *) "ERROR: unknown command\r\n");
        return;
    }

    if (strcmp(cmd, "GET") == 0) {
        if (arg1 == NULL) {
            uart_send_string((uint8_t *) "ERROR: bad arguments\r\n");
            return;
        }
        if (strcmp(arg1, "TMP_MAX") == 0) {
            long_t threshold_int = 0;
            long_t threshold_dec = 0;
            split_int_dec_parts(thermal_fsm_get_threshold(), &threshold_int,
                                &threshold_dec);
            snprintf(msg, sizeof(msg), "TMP_MAX=%ld.%02ld\r\n", threshold_int,
                     threshold_dec);
            uart_send_string((uint8_t *) msg);
            return;
        }
        if (strcmp(arg1, "HYST") == 0) {
            long_t hysteresis_int = 0;
            long_t hysteresis_dec = 0;
            split_int_dec_parts(thermal_fsm_get_hysteresis(), &hysteresis_int,
                                &hysteresis_dec);
            snprintf(msg, sizeof(msg), "HYST=%ld.%02ld\r\n",
                     hysteresis_int, hysteresis_dec);
            uart_send_string((uint8_t *) msg);
            return;
        }
        if (strcmp(arg1, "ALARM_TIMEOUT") == 0) {
            snprintf(msg, sizeof(msg), "ALARM_TIMEOUT=%lu\r\n",
                     (ulong_t) thermal_fsm_get_max_alarm_ms());
            uart_send_string((uint8_t *) msg);
            return;
        }
        if (strcmp(arg1, "STATE") == 0) {
            snprintf(msg, sizeof(msg), "STATE=%s\r\n",
                     thermal_state_to_string(thermal_fsm_get_state()));
            uart_send_string((uint8_t *) msg);
            return;
        }
        if (strcmp(arg1, "TEMP") == 0) {
            long_t temp_int = 0;
            long_t temp_dec = 0;
            split_int_dec_parts(thermal_fsm_get_last_temp(), &temp_int, &temp_dec);
            snprintf(msg, sizeof(msg), "TEMP=%ld.%02ld\r\n", temp_int,
                     temp_dec);
            uart_send_string((uint8_t *) msg);
            return;
        }
        uart_send_string((uint8_t *) "ERROR: unknown command\r\n");
        return;
    }

    uart_send_string((uint8_t *) "ERROR: unknown command\r\n");
}

bool_t uart_send_string(uint8_t *pstring) {
    if (pstring == NULL) {
        return false;
    }
    uint8_t i = 0U;
    while (pstring[i] != STR_END_CHAR && i < UART_STR_MAX_LEN) {
        i++;
    }
    if (i == 0U) {
        return false;
    }
    if (HAL_UART_Transmit(uart_huart, pstring, i, UART_TIMEOUT_MS) != HAL_OK) {
        uart_error_count++;
        return false;
    }
    return true;
}

bool_t uart_send_string_size(uint8_t *pstring, uint16_t size) {
    if (pstring == NULL) {
        return false;
    }
    if (size < UART_SIZE_MIN || size > UART_SIZE_MAX) {
        return false;
    }
    if (HAL_UART_Transmit(uart_huart, pstring, size, UART_TIMEOUT_MS)
        != HAL_OK) {
        uart_error_count++;
        return false;
    }
    return true;
}

bool_t uart_receive_string_size(uint8_t *pstring, uint16_t size) {
    if (pstring == NULL) {
        return false;
    }
    if (size < UART_SIZE_MIN || size > UART_SIZE_MAX) {
        return false;
    }
    if (HAL_UART_Receive(uart_huart, pstring, size, UART_TIMEOUT_MS)
        != HAL_OK) {
        uart_error_count++;
        return false;
    }
    return true;
}

bool_t uart_receive_byte_try(uint8_t *pstring) {
    if (pstring == NULL) {
        return false;
    }

    return (HAL_UART_Receive(uart_huart, pstring, UART_SIZE_MIN, 0U) == HAL_OK) ? true : false;
}

uint32_t uart_get_error_count() {
    return uart_error_count;
}

bool_t uart_init(UART_HandleTypeDef *huart) {
    uart_huart = huart;
    uart_error_count = 0U;

    if (uart_huart == NULL) {
        return false;
    }

    uart_huart->Instance = USART2;
    uart_huart->Init.BaudRate = 115200;
    uart_huart->Init.WordLength = UART_WORDLENGTH_8B;
    uart_huart->Init.StopBits = UART_STOPBITS_1;
    uart_huart->Init.Parity = UART_PARITY_NONE;
    uart_huart->Init.Mode = UART_MODE_TX_RX;
    uart_huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_huart->Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(uart_huart) != HAL_OK) {
        return false;
    }

    uart_send_string((uint8_t *) "\r\n=== api_uart init ===\r\n");
    uart_send_string((uint8_t *) "huart2.Init.BaudRate = 115200\r\n");
    uart_send_string(
        (uint8_t *) "huart2.Init.WordLength = UART_WORDLENGTH_8B\r\n");
    uart_send_string((uint8_t *) "huart2.Init.StopBits = UART_STOPBITS_1\r\n");
    uart_send_string((uint8_t *) "huart2.Init.Parity = UART_PARITY_NONE\r\n");
    uart_send_string((uint8_t *) "huart2.Init.Mode = UART_MODE_TX_RX\r\n");
    uart_send_string(
        (uint8_t *) "huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE\r\n");
    uart_send_string(
        (uint8_t *) "huart2.Init.OverSampling = UART_OVERSAMPLING_16\r\n");
    uart_send_string((uint8_t *) "====================\r\n");

    uart_cli_init();
    uart_send_string((uint8_t *) "\r\nUART ready\r\n");
    uart_send_cmd_help();

    return true;
}

void uart_cli_poll() {
    uint32_t now_ms = HAL_GetTick();
    if ((now_ms - cli_last_report_ms) >= CLI_REPORT_PERIOD_MS) {
        cli_last_report_ms = now_ms;
        uart_send_cmd_temp_report(); // Print current temperature report
    }

    uint8_t byte = 0U;
    while (uart_receive_byte_try(&byte)) {
        // Echo received character back so we can see what we type
        uart_send_string_size(&byte, 1U);

        if ((byte == '\r') || (byte == '\n')) {
            if (cli_index > 0U) {
                // Command line completed, try to process it
                cli_line[cli_index] = STR_END_CHAR;
                process_cli_command((char_t *) cli_line);
                cli_index = 0U;
            }
            continue;
        }

        if (cli_index < (CLI_LINE_MAX - 1U)) {
            // Append character to current line
            cli_line[cli_index++] = byte;
        } else {
            // Line buffer overflow, reset and report error
            cli_index = 0U;
            uart_send_string((uint8_t *) "ERROR: line too long\r\n");
        }
    }
}
