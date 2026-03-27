#ifndef __API_DEBOUNCE_H
#define __API_DEBOUNCE_H
#include "API_delay.h" // solo para bool_t. Revisar sacar a lugar comun

void debounceFSM_init();

void debounceFSM_update();

bool_t readKey();

#endif
