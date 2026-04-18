#ifndef API_BUZZER_H
#define API_BUZZER_H

/* Initialize buzzer output to OFF state. */
void api_buzzer_init(void);

/* Buzzer output to active level (sound ON). */
void api_buzzer_on(void);

/* Buzzer output to inactive level (sound OFF). */
void api_buzzer_off(void);

#endif /* API_BUZZER_H */
