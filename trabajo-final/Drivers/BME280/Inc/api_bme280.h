#ifndef API_BME280_H
#define API_BME280_H

#include "stm32f4xx_hal.h"
#include "types.h"

/**
 * @brief Initialize BME280 over I2C and load temperature calibration data.
 * @param hi2c pointer to I2C handle used by this module
 * @retval true initialization successful
 * @retval false communication/configuration error
 */
bool_t bme280_init(I2C_HandleTypeDef * hi2c);

/**
 * @brief Read compensated temperature in Celsius.
 * @param temp_c pointer where temperature is written
 * @retval true read successful
 * @retval false read/compensation error
 */
bool_t bme280_read_temperature(float_t * temp_c);

#endif /* API_BME280_H */
