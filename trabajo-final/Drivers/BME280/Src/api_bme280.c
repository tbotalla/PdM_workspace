#include "../Inc/api_bme280.h"

#include "stm32f4xx_hal.h"

#include <stddef.h>

#define BME280_ADDR (0x76U << 1) // 7-bit I2C slave address for BME280 (with SDO pin to GND)
#define BME280_CHIP_ID 0x60U // Expected fixed "id" register value for BME280
#define BME280_REG_CHIP_ID 0xD0U // "id" register
#define BME280_REG_CTRL_MEAS 0xF4U // "ctrl_meas" register
#define BME280_REG_TEMP_MSB 0xFAU // "temp_msb" register (temperature data MSB)
#define BME280_REG_DIG_T1 0x88U // "dig_T1" calibration register start address

// ctrl_meas = 0x23 (00100011):
//		bits [7:5] osrs_t = 001 (temperature oversampling x1, no multiple reads for avg)
//		bits [4:2] osrs_p = 000 (pressure skipped)
//		bits [1:0] mode = 11 (normal mode)
#define BME280_CTRL_MEAS_TEMP_X1_NORMAL 0x23U
#define BME280_I2C_TIMEOUT_MS 100U

static uint16_t bme280_dig_t1 = 0U;
static int16_t bme280_dig_t2 = 0;
static int16_t bme280_dig_t3 = 0;
static I2C_HandleTypeDef *bme280_i2c = NULL;

static HAL_StatusTypeDef bme280_read(uint8_t reg, uint8_t *data, uint16_t len) {
    if (bme280_i2c == NULL) {
        return HAL_ERROR;
    }
    return HAL_I2C_Mem_Read(bme280_i2c, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                            data, len, BME280_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef bme280_write(uint8_t reg, uint8_t value) {
    if (bme280_i2c == NULL) {
        return HAL_ERROR;
    }
    return HAL_I2C_Mem_Write(bme280_i2c, BME280_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             &value, 1U, BME280_I2C_TIMEOUT_MS);
}

bool_t bme280_init(I2C_HandleTypeDef *hi2c) {
    bme280_i2c = hi2c;
    if (bme280_i2c == NULL) {
        return false;
    }

    uint8_t chip_id = 0U;
    if (bme280_read(BME280_REG_CHIP_ID, &chip_id, 1U) != HAL_OK) {
        return false;
    }
    if (chip_id != BME280_CHIP_ID) {
        return false;
    }

    // osrs_t=x1, osrs_p=skip, mode=normal
    if (bme280_write(BME280_REG_CTRL_MEAS, BME280_CTRL_MEAS_TEMP_X1_NORMAL) != HAL_OK) {
        return false;
    }

    uint8_t calib[6];
    // Read 6 bytes from dig_T1, so we get dig_T2 and dig_T3 as well
    if (bme280_read(BME280_REG_DIG_T1, calib, sizeof(calib)) != HAL_OK) {
        return false;
    }
    // Split the 2 bytes for each one
    bme280_dig_t1 = (uint16_t)((uint16_t) calib[1] << 8 | calib[0]); // calib[0] = LSB, calib[1] = MSB
    bme280_dig_t2 = (int16_t)((uint16_t) calib[3] << 8 | calib[2]); // calib[2] = LSB, calib[3] = MSB
    bme280_dig_t3 = (int16_t)((uint16_t) calib[5] << 8 | calib[4]); // calib[4] = LSB, calib[] = MSB

    return true;
}

bool_t bme280_read_temperature(float_t *temp_c) {
    if (temp_c == NULL) {
        return false;
    }

    uint8_t raw[3];
    // Reads 3 bytes: 0xFA (temp_msb, [3:0]), 0xFB (temp_lsb, [11:4]), 0xFC (temp_xlsb, [19:12])
    // The sensor uses only 20 of those bits
    if (bme280_read(BME280_REG_TEMP_MSB, raw, sizeof(raw)) != HAL_OK) {
        return false;
    }

    // raw[0] << 12 : bits [19:12]
    // raw[1] << 4  : bits [11:4]
    // raw[2] >> 4  : bits [3:0]
    int32_t adc_t = (int32_t)((((uint32_t) raw[0]) << 12)
                              | (((uint32_t) raw[1]) << 4) | (((uint32_t) raw[2]) >> 4));

    // Compensation formula extracted from Bosch datasheet
    int32_t var1 = ((((adc_t >> 3) - ((int32_t) bme280_dig_t1 << 1)))
                    * ((int32_t) bme280_dig_t2)) >> 11;
    int32_t var2 = (((((adc_t >> 4) - ((int32_t) bme280_dig_t1))
                      * ((adc_t >> 4) - ((int32_t) bme280_dig_t1))) >> 12)
                    * ((int32_t) bme280_dig_t3)) >> 14;
    int32_t t_fine = var1 + var2;

    int32_t temp_x100 = (t_fine * 5 + 128) >> 8;
    *temp_c = ((float_t) temp_x100) / 100.0f; // Output the value
    return true;
}
