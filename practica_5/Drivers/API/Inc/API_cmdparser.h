#ifndef API_CMDPARSER_H
#define API_CMDPARSER_H

#include <stdint.h>

/**
 * Maximum command line length
 */
#define CMD_MAX_LINE 64U

/* Parser status/error codes */
typedef enum {
	CMD_OK = 0,
	CMD_ERR_OVERFLOW,
	CMD_ERR_SYNTAX,
	CMD_ERR_UNKNOWN,
	CMD_ERR_ARG
} cmd_status_t;

/* Initializes parser state and internal buffers */
void cmdParserInit(void);

/**
 * @brief Polls UART and advances the parser FSM.
 */
void cmdPoll(void);

/* Prints the available command list over UART. */
void cmdPrintHelp(void);

#endif
