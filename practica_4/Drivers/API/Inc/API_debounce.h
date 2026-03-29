#ifndef __API_DEBOUNCE_H
#define __API_DEBOUNCE_H

#include <API_delay.h>

/**
 * @brief Initialize the de-bounce state machine with an initial
 * button un-pressed state.
 */
void debounceFSM_init();

/**
 * @brief Updates the state machine based on the status of the pin read*/
void debounceFSM_update();

/**
 * @brief Returns the current state of the button. The returned value is
 * reset to false immediately after its read.
 * @return true when the button was pressed, false if not.
 * */
bool_t readKey();

#endif
