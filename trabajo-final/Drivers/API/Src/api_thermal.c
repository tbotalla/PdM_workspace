#include "api_thermal.h"

#include "api_bme280.h"
#include "api_buzzer.h"

#include "stm32f4xx_hal.h"

static thermal_state_t thermal_state = thermal_state_init;
static float_t threshold_c = 25.0f;
static float_t hysteresis_c = 1.0f;
static float_t last_temp_c = 0.0f;
static uint32_t max_alarm_ms = 10000U;
static uint32_t last_sample_ms = 0U;
static uint32_t alarm_start_ms = 0U;
static uint32_t last_recovery_try_ms = 0U;
static I2C_HandleTypeDef *thermal_i2c = NULL;

#define THERMAL_SAMPLE_PERIOD_MS 500U // How often to sample the temperature
#define THERMAL_RECOVERY_PERIOD_MS 1000U // ms to wait for recovery after sensor error
#define THRESHOLD_MIN_C (-40.0f)
#define THRESHOLD_MAX_C (85.0f)
#define HYSTERESIS_MIN_C (0.1f)
#define HYSTERESIS_MAX_C (20.0f)
#define MAX_ALARM_MIN_MS 1000U
#define MAX_ALARM_MAX_MS 120000U

/* Limit the float_t value to be between min_value & max_value if outside range */
static float_t bound_float(float_t value, float_t min_value, float_t max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

/* Limit the uint32_t value to be between min_value & max_value if outside range */
static uint32_t bound_u32(uint32_t value, uint32_t min_value,
                          uint32_t max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

void thermal_fsm_init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == NULL) {
        return;
    }
    thermal_i2c = hi2c;

    // Default initial values
    threshold_c = 25.0f;
    hysteresis_c = 1.0f;
    max_alarm_ms = 10000U;
    last_temp_c = 0.0f;
    last_sample_ms = 0U;
    alarm_start_ms = 0U;
    last_recovery_try_ms = 0U;

    // Init buzzer
    buzzer_init();
    buzzer_off();

    // First transition to MONITORING or SENSOR_ERROR happens in thermal_fsm_update
    thermal_state = thermal_state_init;
}

void thermal_fsm_update(void) {
    uint32_t now_ms = HAL_GetTick();
    float_t alarm_off_c = threshold_c - hysteresis_c;
    if (thermal_state == thermal_state_alarm_on) {
        if (now_ms - alarm_start_ms >= max_alarm_ms) {
            // Max alarm time exceeded, transition to thermal_state_alarm_timeout
            buzzer_off();
            thermal_state = thermal_state_alarm_timeout;
        }
    }

    float_t temp_c = 0.0f;
    bool_t new_sample_ready = false;

    // Do not sample before BME280 is configured (first bme280_init runs in case thermal_state_init)
    if (thermal_state != thermal_state_init) {
        if (now_ms - last_sample_ms >= THERMAL_SAMPLE_PERIOD_MS) {
            // Sample period elapsed, read temperature
            last_sample_ms = now_ms;
            if (bme280_read_temperature(&temp_c)) {
                last_temp_c = temp_c;
                new_sample_ready = true;
            } else {
                buzzer_off();
                // Error reading temperature, transition to thermal_state_sensor_error
                thermal_state = thermal_state_sensor_error;
            }
        }
    }

    switch (thermal_state) {
        case thermal_state_init:
            // Depending on bme280_init result, transition to thermal_state_sensor_error or thermal_state_monitoring if ok
            buzzer_off();
            thermal_state = bme280_init(thermal_i2c) ? thermal_state_monitoring : thermal_state_sensor_error;
            break;

        case thermal_state_monitoring:
            buzzer_off();
            if (new_sample_ready && last_temp_c >= threshold_c) {
                // Temperature above threshold, transition to thermal_state_alarm_on
                thermal_state = thermal_state_alarm_on;
                alarm_start_ms = now_ms;
                buzzer_on();
            }
            break;

        case thermal_state_alarm_on:
            if (new_sample_ready && last_temp_c <= alarm_off_c) {
                // Temperature below alarm off threshold, transition to thermal_state_monitoring
                buzzer_off();
                thermal_state = thermal_state_monitoring;
                break;
            }
            buzzer_on();
            break;

        case thermal_state_alarm_timeout:
            buzzer_off();
            if (new_sample_ready && last_temp_c <= alarm_off_c) {
                // Temperature below alarm off threshold, transition to thermal_state_monitoring
                thermal_state = thermal_state_monitoring;
            }
            break;

        case thermal_state_sensor_error:
            buzzer_off();
            // Try to recover after a sensor error
            if (now_ms - last_recovery_try_ms >= THERMAL_RECOVERY_PERIOD_MS) {
                last_recovery_try_ms = now_ms;
                if (bme280_init(thermal_i2c)) {
                    // Sensor recovered, transition to thermal_state_monitoring
                    thermal_state = thermal_state_monitoring;
                }
            }
            break;

        default:
            // Unknown state, transition to thermal_state_sensor_error
            buzzer_off();
            thermal_state = thermal_state_sensor_error;
            break;
    }
}

void thermal_fsm_set_threshold(float_t threshold_value_c) {
    threshold_c = bound_float(threshold_value_c, THRESHOLD_MIN_C, THRESHOLD_MAX_C);
}

float_t thermal_fsm_get_threshold(void) {
    return threshold_c;
}

void thermal_fsm_set_hysteresis(float_t hysteresis_value_c) {
    hysteresis_c = bound_float(hysteresis_value_c, HYSTERESIS_MIN_C,
                               HYSTERESIS_MAX_C);
}

float_t thermal_fsm_get_hysteresis(void) {
    return hysteresis_c;
}

void thermal_fsm_set_max_alarm_ms(uint32_t max_alarm_value_ms) {
    max_alarm_ms = bound_u32(max_alarm_value_ms, MAX_ALARM_MIN_MS,
                             MAX_ALARM_MAX_MS);
}

uint32_t thermal_fsm_get_max_alarm_ms(void) {
    return max_alarm_ms;
}

float_t thermal_fsm_get_last_temp(void) {
    return last_temp_c;
}

thermal_state_t thermal_fsm_get_state(void) {
    return thermal_state;
}

const char_t *thermal_state_to_string(thermal_state_t state) {
    switch (state) {
        case thermal_state_init:
            return "INIT";
        case thermal_state_monitoring:
            return "MONITORING";
        case thermal_state_alarm_on:
            return "ALARM_ON";
        case thermal_state_alarm_timeout:
            return "ALARM_TIMEOUT";
        case thermal_state_sensor_error:
            return "SENSOR_ERROR";
        default:
            return "UNKNOWN";
    }
}
