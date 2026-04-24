#ifndef API_BUZZER_H
#define API_BUZZER_H

/* Initialize buzzer output to OFF state. */
void buzzer_init();

/* Buzzer output to active level (sound ON). */
void buzzer_on();

/* Buzzer output to inactive level (sound OFF). */
void buzzer_off();

#endif /* API_BUZZER_H */
