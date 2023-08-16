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

# I2C Read

```
uint8_t i2c_read(uint8_t reg)
{
  uint8_t data;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd,(ICM_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true);
  i2c_master_start(cmd); 
  i2c_master_write_byte(cmd, (ICM_ADDR << 1) | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, &data, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);

  return data;
}
```
This is similar to the ```i2c_write```, but after writing the register that contains the data we want to read, it sends another start condition and the sensor address, but this time it sends the read bit. Contents of the register are then stored in the data variable. The datasheet (9.4) specifies how to read to the sensor registers via I2C.
```
void read_gyro() {
  int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
  gyro_x_raw = (i2c_read(0x11) << 8) | i2c_read(0x12);
  gyro_y_raw = (i2c_read(0x13) << 8) | i2c_read(0x14);
  gyro_z_raw = (i2c_read(0x15) << 8) | i2c_read(0x16);

  float gyro_x, gyro_y, gyro_z;
  gyro_x = gyro_x_raw / 16.4;
  gyro_y = gyro_y_raw / 16.4;
  gyro_z = gyro_z_raw / 16.4;

...

}
```
To read the gyroscope values, ```read_gyro``` utilises the i2c_read function to read the registers with the relevant data. Each value (x, y, z) is 2 bytes (datasheet 6.1, 15.16, 15,17) so the function combines the values from two registers using bit shifting and the OR operator. The datasheet (14.1) provides the register addresses where the values are stored: 0x11 to 0x16. The gyro is configured with a range of +-2000dps. The corresponding sensitivity scale factor is 16.4 (datasheet 3.1), so the raw value needs to be divided by 16.4 to get the correct measurement. 

```
void read_accel() {
  int16_t acc_x_raw, acc_y_raw, acc_z_raw;
  acc_x_raw = (i2c_read(0x0B) << 8) | i2c_read(0x0C);
  acc_y_raw = (i2c_read(0x0D) << 8) | i2c_read(0x0E);
  acc_z_raw = (i2c_read(0x0F) << 8) | i2c_read(0x10);

  float acc_x, acc_y, acc_z;
  acc_x = (float) acc_x_raw / 2048;
  acc_y = (float) acc_y_raw / 2048;
  acc_z = (float) acc_z_raw / 2048;

...

}
```
```read_acel```is basically the same as ```read_gyro```. The only differences are the register addresses for the accelerometer values and the sensitivity scale factor.
