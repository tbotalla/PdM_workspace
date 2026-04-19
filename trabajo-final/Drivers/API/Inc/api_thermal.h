#ifndef API_THERMAL_H
#define API_THERMAL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "types.h"

typedef enum {
    thermal_state_init = 0,
    thermal_state_monitoring,
    thermal_state_alarm_on,
    thermal_state_alarm_timeout,
    thermal_state_sensor_error
} thermal_state_t;

/**
 * @brief Reset parameters, init buzzer and I2C handle. FSM stays in thermal_state_init until the first thermal_fsm_update.
 * @param hi2c pointer to I2C handle for sensor communication
 */
void thermal_fsm_init(I2C_HandleTypeDef * hi2c);

/**
 * @brief Run one cycle of the thermal FSM (call this one periodically from main loop).
 */
void thermal_fsm_update();

/**
 * @brief Set temperature threshold for alarm activation
 * @param threshold_c threshold value in Celsius
 */
void thermal_fsm_set_threshold(float_t threshold_c);

/**
 * @brief Get current temperature threshold
 * @retval threshold value in Celsius
 */
float_t thermal_fsm_get_threshold();

/**
 * @brief Set hysteresis for alarm deactivation
 * @param hysteresis_c hysteresis value in Celsius
 */
void thermal_fsm_set_hysteresis(float_t hysteresis_c);

/**
 * @brief Get current hysteresis value
 * @retval hysteresis value in Celsius
 */
float_t thermal_fsm_get_hysteresis();

/**
 * @brief Set maximum alarm duration before entering timeout state
 * @param max_alarm_ms duration in milliseconds
 */
void thermal_fsm_set_max_alarm_ms(uint32_t max_alarm_ms);

/**
 * @brief Get current maximum alarm duration
 * @retval duration in milliseconds
 */
uint32_t thermal_fsm_get_max_alarm_ms();

/**
 * @brief Get last temperature reading from the sensor
 * @retval temperature in Celsius
 */
float_t thermal_fsm_get_last_temp();

/**
 * @brief Get current FSM state
 * @retval current thermal_state_t value
 */
thermal_state_t thermal_fsm_get_state();

/**
 * @brief Convert FSM state to a human-readable string
 * @param state FSM state to convert
 * @retval pointer to static string representation
 */
const char_t *thermal_state_to_string(thermal_state_t state);

#endif /* API_THERMAL_H */
