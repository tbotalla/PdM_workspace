#include "api_buzzer.h"
#include "driver.h"

void buzzer_init() {
    driver_buzzer_init();
}

void buzzer_on() {
    driver_buzzer_on();
}

void buzzer_off() {
    driver_buzzer_off();
}
