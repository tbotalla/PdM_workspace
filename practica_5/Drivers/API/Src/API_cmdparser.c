#include "API_cmdparser.h"

#include "API_uart.h"
#include "main.h"

#define CHAR_CR '\r'
#define CHAR_LF '\n'
#define CHAR_END_STR '\0'
#define CHAR_COMMENT_HASH '#'
#define CHAR_COMMENT_SLASH '/'
#define CHAR_SPACE ' '
#define CHAR_TAB '\t'
#define CMD_SINGLE_BYTE 1U

// Messages
#define CMD_MSG_ERROR_LINE_TOO_LONG "ERROR: line too long\r\n"
#define CMD_MSG_ERROR_UNKNOWN_COMMAND "ERROR: unknown command\r\n"
#define CMD_MSG_ERROR_BAD_ARGUMENTS "ERROR: bad arguments\r\n"
#define CMD_MSG_STATUS_ON "LED is ON\r\n"
#define CMD_MSG_STATUS_OFF "LED is OFF\r\n"
#define CMD_MSG_HELP "HELP\r\nLED ON\r\nLED OFF\r\nLED TOGGLE\r\nSTATUS\r\n"

// Commands
#define CMD_WORD_HELP "HELP"
#define CMD_WORD_STATUS "STATUS"
#define CMD_WORD_LED "LED"
#define CMD_WORD_LED_ON "LED ON"
#define CMD_WORD_LED_OFF "LED OFF"
#define CMD_WORD_LED_TOGGLE "LED TOGGLE"

// State machine representing the command parsing status
typedef enum {
	CMD_STATE_IDLE = 0,
	CMD_STATE_RECEIVING,
	CMD_STATE_PROCESS,
	CMD_STATE_EXEC,
	CMD_STATE_ERROR
} cmd_state_t;

// Actions to execute
typedef enum {
	CMD_ACTION_NONE = 0,
	CMD_ACTION_HELP,
	CMD_ACTION_STATUS,
	CMD_ACTION_LED_ON,
	CMD_ACTION_LED_OFF,
	CMD_ACTION_LED_TOGGLE
} cmd_action_t;

static cmd_state_t cmd_state = CMD_STATE_IDLE;
static cmd_status_t last_status = CMD_OK;
static cmd_action_t pending_action = CMD_ACTION_NONE;
static const uint8_t *pending_error_message = NULL;
static uint8_t line_buffer[CMD_MAX_LINE + CMD_SINGLE_BYTE]; // 64B + 1B (\0)
static uint16_t line_length = 0U;

static void cmd_process_line(uint8_t *line);
static void cmd_exec_action(void);
static void reset_line_buffer(void);
static uint8_t* left_trim_whitespaces(uint8_t *text);
static bool_t is_separator(uint8_t ch);
static uint8_t char_to_lower(uint8_t ch);
static bool_t str_equals_ignore_case(const uint8_t *left, const uint8_t *right);
static uint8_t* next_token(uint8_t **cursor);
static bool_t append_char_to_line(uint8_t ch);
static bool_t is_two_word_command(const uint8_t *token_1,
		const uint8_t *token_2, const uint8_t *full_command);

/* Initializes parser state and internal buffers. */
void cmdParserInit(void) {
	cmd_state = CMD_STATE_IDLE;
	last_status = CMD_OK;
	pending_action = CMD_ACTION_NONE;
	pending_error_message = NULL;
	reset_line_buffer();
}

/* Prints the supported command list over UART. */
void cmdPrintHelp(void) {
	uartSendString((uint8_t*) CMD_MSG_HELP);
}

/* Polls UART and advances the command parser state machine */
void cmdPoll(void) {
	uint8_t rx_char = 0U;
	bool_t has_rx_char = false;

	if (cmd_state == CMD_STATE_IDLE || cmd_state == CMD_STATE_RECEIVING) {
		has_rx_char = uartReceiveByteTry(&rx_char); // Controls if we actually received something
		if (has_rx_char) {
			uartSendStringSize(&rx_char, CMD_SINGLE_BYTE);
		}
	}

	switch (cmd_state) {
	case CMD_STATE_IDLE:
		if (!has_rx_char) {
			break; // No char received, nothing to do
		}
		if (rx_char == CHAR_CR || rx_char == CHAR_LF) {
			break; // Line break, nothing to do
		}
		if (!append_char_to_line(rx_char)) {
			// State transition to CMD_STATE_ERROR: char exceeded max line length
			last_status = CMD_ERR_OVERFLOW;
			pending_error_message =
					(const uint8_t*) CMD_MSG_ERROR_LINE_TOO_LONG;
			cmd_state = CMD_STATE_ERROR;
		} else {
			// State transition to CMD_STATE_RECEIVING: char appended to line buffer
			cmd_state = CMD_STATE_RECEIVING;
		}
		break;

	case CMD_STATE_RECEIVING:
		if (!has_rx_char) {
			break; // No char received, nothing to do
		}
		if (rx_char == CHAR_CR || rx_char == CHAR_LF) {
			// State transition to CMD_STATE_PROCESS: received a line break
			cmd_state = CMD_STATE_PROCESS;
		} else if (!append_char_to_line(rx_char)) {
			// State transition to CMD_STATE_ERROR: char exceeded max line length
			last_status = CMD_ERR_OVERFLOW;
			pending_error_message =
					(const uint8_t*) CMD_MSG_ERROR_LINE_TOO_LONG;
			cmd_state = CMD_STATE_ERROR;
		}
		break;

	case CMD_STATE_PROCESS:
		line_buffer[line_length] = CHAR_END_STR; // Appends string end char at the end
		cmd_process_line(line_buffer); // Attempts to process the line command, which will update last_status var
		if (last_status == CMD_OK) {
			// State transition to CMD_STATE_EXEC: command is valid
			cmd_state = CMD_STATE_EXEC;
		} else {
			// State transition to CMD_STATE_ERROR: command is invalid
			cmd_state = CMD_STATE_ERROR;
		}
		break;

	case CMD_STATE_EXEC:
		cmd_exec_action(); // Executes the action
		reset_line_buffer();
		pending_action = CMD_ACTION_NONE;
		cmd_state = CMD_STATE_IDLE; // State transition to CMD_STATE_IDLE: command successfully processed
		break;

	case CMD_STATE_ERROR:
		if (pending_error_message != NULL) {
			// Sends the stored error message
			uartSendString((uint8_t*) pending_error_message);
		}
		reset_line_buffer();
		pending_error_message = NULL;
		// State transition to CMD_STATE_IDLE: back to accepting new commands
		cmd_state = CMD_STATE_IDLE;
		break;

	default:
		cmd_state = CMD_STATE_IDLE;
		reset_line_buffer();
		break;
	}
}

/* Validates one full line and maps it to an action or an error */
static void cmd_process_line(uint8_t *line) {
	uint8_t *trimmed_line = left_trim_whitespaces(line);
	uint8_t *cursor = NULL;
	uint8_t *token_1 = NULL;
	uint8_t *token_2 = NULL;
	uint8_t *token_3 = NULL;

	last_status = CMD_OK;
	pending_action = CMD_ACTION_NONE;
	pending_error_message = NULL;

	// Ignore empty lines or starting with comments
	if (trimmed_line[0] == CHAR_END_STR) {
		return;
	}
	if (trimmed_line[0] == CHAR_COMMENT_HASH) {
		return;
	}
	if (trimmed_line[0] == CHAR_COMMENT_SLASH
			&& trimmed_line[1] == CHAR_COMMENT_SLASH) {
		return;
	}

	// Parse line tokens (words)
	cursor = trimmed_line;
	token_1 = next_token(&cursor); // First word
	token_2 = next_token(&cursor); // Second word

	if (token_1 == NULL) {
		last_status = CMD_ERR_SYNTAX;
		pending_error_message = (const uint8_t*) CMD_MSG_ERROR_BAD_ARGUMENTS;
		return;
	}

	if (str_equals_ignore_case(token_1, (const uint8_t*) CMD_WORD_HELP)) {
		if (token_2 != NULL) {
			// HELP command with args is invalid
			last_status = CMD_ERR_ARG;
			pending_error_message =
					(const uint8_t*) CMD_MSG_ERROR_BAD_ARGUMENTS;
			return;
		}
		pending_action = CMD_ACTION_HELP;
		return;
	}

	if (str_equals_ignore_case(token_1, (const uint8_t*) CMD_WORD_STATUS)) {
		if (token_2 != NULL) {
			// STATUS command with args is invalid
			last_status = CMD_ERR_ARG;
			pending_error_message =
					(const uint8_t*) CMD_MSG_ERROR_BAD_ARGUMENTS;
			return;
		}
		pending_action = CMD_ACTION_STATUS;
		return;
	}

	if (str_equals_ignore_case(token_1, (const uint8_t*) CMD_WORD_LED)) {
		if (token_2 == NULL) {
			// LED command without args is invalid
			last_status = CMD_ERR_ARG;
			pending_error_message =
					(const uint8_t*) CMD_MSG_ERROR_BAD_ARGUMENTS;
			return;
		}
		if (is_two_word_command(token_1, token_2,
				(const uint8_t*) CMD_WORD_LED_ON)) {
			pending_action = CMD_ACTION_LED_ON;
			return;
		}
		if (is_two_word_command(token_1, token_2,
				(const uint8_t*) CMD_WORD_LED_OFF)) {
			pending_action = CMD_ACTION_LED_OFF;
			return;
		}
		if (is_two_word_command(token_1, token_2,
				(const uint8_t*) CMD_WORD_LED_TOGGLE)) {
			pending_action = CMD_ACTION_LED_TOGGLE;
			return;
		}

		// LED + XYZ is invalid when XYZ != [ON, OFF, TOGGLE]
		last_status = CMD_ERR_ARG;
		pending_error_message = (const uint8_t*) CMD_MSG_ERROR_BAD_ARGUMENTS;
		return;
	}

	last_status = CMD_ERR_UNKNOWN;
	pending_error_message = (const uint8_t*) CMD_MSG_ERROR_UNKNOWN_COMMAND;
}

/* Executes the pending parsed action */
static void cmd_exec_action(void) {
	switch (pending_action) {
	case CMD_ACTION_HELP:
		cmdPrintHelp();
		break;

	case CMD_ACTION_STATUS:
		if (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET) {
			uartSendString((uint8_t*) CMD_MSG_STATUS_ON);
		} else {
			uartSendString((uint8_t*) CMD_MSG_STATUS_OFF);
		}
		break;

	case CMD_ACTION_LED_ON:
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		break;

	case CMD_ACTION_LED_OFF:
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		break;

	case CMD_ACTION_LED_TOGGLE:
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		break;

	case CMD_ACTION_NONE:
	default:
		break;
	}
}

/* Clears the command line buffer */
static void reset_line_buffer(void) {
	line_length = 0U;
	line_buffer[0] = CHAR_END_STR;
}

/* Appends one character if there is free space in the buffer */
static bool_t append_char_to_line(uint8_t ch) {
	if (line_length >= CMD_MAX_LINE) {
		return false;
	}
	line_buffer[line_length++] = ch;
	line_buffer[line_length] = CHAR_END_STR;
	return true;
}

/* Skips leading spaces and tabs. */
static uint8_t* left_trim_whitespaces(uint8_t *text) {
	while (text[0] == CHAR_SPACE || text[0] == CHAR_TAB) {
		text++;
	}
	return text;
}

/* Returns true for token separators (spaces or tabs) */
static bool_t is_separator(uint8_t ch) {
	return (ch == CHAR_SPACE || ch == CHAR_TAB) ? true : false;
}

/* Converts one ASCII letter to lowercase */
static uint8_t char_to_lower(uint8_t ch) {
	if (ch >= (uint8_t) 'A' && ch <= (uint8_t) 'Z') {
		return (uint8_t) (ch + ((uint8_t) 'a' - (uint8_t) 'A'));
	}
	return ch;
}

/* Compares two strings case-insensitive */
static bool_t str_equals_ignore_case(const uint8_t *left, const uint8_t *right) {
	uint16_t index = 0U;

	if (left == NULL || right == NULL) {
		return false;
	}
	while (left[index] != CHAR_END_STR && right[index] != CHAR_END_STR) {
		if (char_to_lower(left[index]) != char_to_lower(right[index])) {
			return false;
		}
		index++;
	}
	return (left[index] == CHAR_END_STR && right[index] == CHAR_END_STR) ?
			true : false;
}

/* Returns the next token (word separated by spaces/tabs) and advances the cursor
 * Receives a uint8_t** so the caller can see the cursor updated from the outside
 * */
static uint8_t* next_token(uint8_t **cursor) {
	uint8_t *start = NULL;

	if (cursor == NULL || *cursor == NULL) {
		return NULL;
	}
	while (is_separator((*cursor)[0])) {
		(*cursor)++;
	}
	if ((*cursor)[0] == CHAR_END_STR) {
		return NULL;
	}

	start = *cursor;
	while ((*cursor)[0] != CHAR_END_STR && !is_separator((*cursor)[0])) {
		(*cursor)++;
	}
	if ((*cursor)[0] != CHAR_END_STR) {
		(*cursor)[0] = CHAR_END_STR;
		(*cursor)++;
	}
	return start;
}

/* Checks if token_1 + token_2 match a two-word command. */
static bool_t is_two_word_command(const uint8_t *token_1,
		const uint8_t *token_2, const uint8_t *full_command) {
	uint8_t word_1[CMD_MAX_LINE + CMD_SINGLE_BYTE];
	uint8_t word_2[CMD_MAX_LINE + CMD_SINGLE_BYTE];
	uint16_t index = 0U;
	uint16_t word_1_index = 0U;
	uint16_t word_2_index = 0U;

	if (token_1 == NULL || token_2 == NULL || full_command == NULL) {
		return false;
	}

	while (full_command[index] != CHAR_END_STR
			&& full_command[index] != CHAR_SPACE
			&& word_1_index < CMD_MAX_LINE) {
		word_1[word_1_index++] = full_command[index++];
	}
	word_1[word_1_index] = CHAR_END_STR;

	if (full_command[index] != CHAR_SPACE) {
		return false;
	}
	index++;

	while (full_command[index] != CHAR_END_STR
			&& word_2_index < CMD_MAX_LINE) {
		word_2[word_2_index++] = full_command[index++];
	}
	word_2[word_2_index] = CHAR_END_STR;

	return (str_equals_ignore_case(token_1, word_1)
			&& str_equals_ignore_case(token_2, word_2)) ? true : false;
}
