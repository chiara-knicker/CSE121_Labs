#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

static const char *TAG = "lec2";

#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number
#define I2C_MASTER_SCL_IO 8 // GPIO number for I2C clock
#define I2C_MASTER_SDA_IO 10 // GPIO number for I2C data
#define I2C_MASTER_FREQ_HZ 100000 // I2C master clock frequency

#define ICM_ADDR 0x68 // I2C address of the ICM-42670-P sensor

static esp_err_t i2c_master_init();

void i2c_write(uint8_t reg, uint8_t data);

void icm_init();
  // PWR_MGMT0 register, gyro and accel in LN mode and RC oscillator on

void acc_conf();
  // ACCEL_CONFIG0 register, set full-scale range to +/-16g 

void gyr_conf();
  // GYRO_CONFIG0 register, set full-scale range to +/-2000dps


uint8_t i2c_read(uint8_t reg);

void read_gyro();

void read_accel();
