#ifndef I2C_API_H
#define I2C_ACCEL_API_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

// For I2C
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number
#define I2C_MASTER_SCL_IO 8 // GPIO number for I2C master SCL (clock)
#define I2C_MASTER_SDA_IO 10 // GPIO number for I2C master SDA (data)
#define I2C_MASTER_FREQ_HZ 100000 // I2C master clock frequency

// For temperature and humidity communication
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define SHTC3_SENSOR_ADDR 0x70 /*!< SHTC3 12C address */
#define SHTC3_CMD_READ_ID OxEFCS8
//#define SHTC3_CMD_MEASURE 0x7866
#define SHTC3_CMD_MEASURE 0x7CA2

// For accelerometer and gyroscope communication
#define ICM_ADDR 0x68 // I2C address of the ICM-42670-P sensor

esp_err_t i2c_master_init();

// --------------------- Temperature and Humidity -----------------------

void i2c_write_shtc3(uint16_t command);

void i2c_read_shtc3(uint8_t *data, size_t size);

float calculate_humidity(uint16_t raw_humidity);

float calculate_temp(uint16_t raw_temp);

float calculate_temp_f(uint16_t raw_temp);

float* read_temp_hum();

// --------------------- Accelerometer and Gyroscope -----------------------

void i2c_write_icm(uint8_t reg, uint8_t data);

void icm_init();

void acc_conf();

void gyr_conf();

uint8_t i2c_read_icm(uint8_t reg);

float* read_gyro();

float* read_accel();

#endif

