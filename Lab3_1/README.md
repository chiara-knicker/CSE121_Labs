# Lab 3.1
# Importing Libraries
```
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"
```
# I2C Configuration
```
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number
#define I2C_MASTER_SCL_IO 8 // GPIO number for I2C clock
#define I2C_MASTER_SDA_IO 10 // GPIO number for I2C data
#define I2C_MASTER_FREQ_HZ 100000 // I2C master clock frequency
#define ICM_ADDR 0x68 // I2C address of the ICM-42670-P sensor
```
# I2C Initializeation
```
i2c_config_t conf = {
  	.mode = I2C_MODE_MASTER,
  	.sda_io_num = I2C_MASTER_SDA_IO,
  	.sda_pullup_en = GPIO_PULLUP_ENABLE,
  	.scl_io_num = I2C_MASTER_SCL_IO,
  	.scl_pullup_en = GPIO_PULLUP_ENABLE,
  	.master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
  if (err != ESP_OK) {
	  ESP_LOGE(TAG, "Failed to i2c_param_config %d", err);
	  return err;
  }

  err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
  if (err != ESP_OK) {
	  ESP_LOGE(TAG, "Failed to i2c_driver_install %d", err);
	  return err;
  }
  return err;
  ```
  # I2C Write
  ```
  void i2c_write(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true); // ACCEL_CONFIG0 register
    i2c_master_write_byte(cmd, data, true); // Set full-scale range to +/-16g
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(2000));
    i2c_cmd_link_delete(cmd);
  ```
This functions takes the register to write to and the data that should be written to that register as an input. The datasheet (9.4) specifies how to write to the sensor registers via I2C. First, the master sends the start condition, followed by the sensor address and the write bit. The master then puts the register to write to on the bus and after receiving an acknowledgement, it puts the data to write on the bus. To end the communication, it sends the stop condition.
```
void icm_init() {
  // PWR_MGMT0 register, gyro and eccel in LN mode and RC oscillator on
  i2c_write(0x1F, 0b00011111);
}
```
This initializes the sensor. The accelerometer and gyroscope are initially powered off, so we need to turn them on. For this, we write to the PWR_MGMT0 register. The datasheet (14.1, 15.26) explains what this register is for and what values to write to it. 

- bit 7 - ACCEL_LP_CLK_SEL: we set this to 0 as this is the recommended setting
- bit 6:5: reserved, so we set it to 0
- bit 4 - IDLE: we set this to 1, but since we use the accelerometer in LN not LP mode, it could also be 0 (datasheet (13) specifies it has to be 1 if in LP mode)
- bit 3:2 - GYRO_MODE: we set this to 11 to place the gyroscope in Low Noise (LN) Mode
- bit 1:0 - ACCEL_MODE: we set this to 11 to place the accelerometer in Low Noise (LN) Mode
```
void acc_conf() {
  // ACCEL_CONFIG0 register, set full-scale range to +/-16g 
  i2c_write(0x21, 0b00011111);
}
```
This configures the accelerometer by writing to the ACCEL_CONFIG0 register. 
```
void gyr_conf() {
  // GYRO_CONFIG0 register, set full-scale range to +/-2000dps
  i2c_write(0x20, 0b00001001);
}
```
This configures the gyroscope by writing to the GYRO_CONFIG0 register. 
