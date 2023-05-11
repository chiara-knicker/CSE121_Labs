#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

#include "i2c_api.h"

static const char *TAG = "lec2";

esp_err_t i2c_master_init()
{
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
}

// --------------------- Temperature and Humidity -----------------------

void i2c_write_shtc3(uint16_t command)
{    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR <<1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, command >> 8, true);
    i2c_master_write_byte(cmd, command & 0xFF, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to 1st write %d", err);
    }

    vTaskDelay(pdMS_TO_TICKS(20));
}

void i2c_read_shtc3(uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to 2nd read %d", err);
    }
}

float calculate_humidity(uint16_t raw_humidity)
{
    return 100.0 * (float)raw_humidity / 65535.0;
}

float calculate_temp(uint16_t raw_temp)
{
    return -45 + 175 * raw_temp / 65535.0;
}

float calculate_temp_f(uint16_t raw_temp)
{
    return calculate_temp(raw_temp) * 1.8 + 32;
}

float* read_temp_hum()
{
	uint8_t data[6] = {0,};

      i2c_write_shtc3(SHTC3_CMD_MEASURE);
	i2c_read_shtc3(data, 6);

      uint16_t raw_humidity, raw_temp;
	raw_humidity = (data[3] << 8) | data[4];
	raw_temp = (data[0] << 8) | data[1];

	float humidity, temp, temp_f;
      humidity = calculate_humidity(raw_humidity);
	temp = calculate_temp(raw_temp);
	temp_f = calculate_temp_f(raw_temp);

	printf("Temperature is %.2fC (or %.2fF) with a %.2f %%  humidity\n", temp, temp_f, humidity);

	float* values = (float*) malloc(3 * sizeof(float));
	values[0] = temp;
	values[1] = temp_f;
	values[2] = humidity;

	return values;
}

// --------------------- Accelerometer and Gyroscope -----------------------

void i2c_write_icm(uint8_t reg, uint8_t data)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (ICM_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true);
  i2c_master_write_byte(cmd, data, true); 
  i2c_master_stop(cmd);
  esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(2000));
  i2c_cmd_link_delete(cmd);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write to I2C\n");
    ESP_LOGE(TAG, "Error: %d\n", err);
  }
}

void icm_init()
{
  // PWR_MGMT0 register, gyro and eccel in LN mode and RC oscillator on
  i2c_write_icm(0x1F, 0b00011111);
}

void acc_conf()
{
  // ACCEL_CONFIG0 register, set full-scale range to +/-16g 
  i2c_write_icm(0x21, 0b00011111);
}

void gyr_conf()
{
  // GYRO_CONFIG0 register, set full-scale range to +/-2000dps
  i2c_write_icm(0x20, 0b00001001);
}


uint8_t i2c_read_icm(uint8_t reg)
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

float*  read_gyro()
{
  int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
  gyro_x_raw = (i2c_read_icm(0x11) << 8) | i2c_read_icm(0x12);
  gyro_y_raw = (i2c_read_icm(0x13) << 8) | i2c_read_icm(0x14);
  gyro_z_raw = (i2c_read_icm(0x15) << 8) | i2c_read_icm(0x16);

  float gyro_x, gyro_y, gyro_z;
  gyro_x = gyro_x_raw / 16.4;
  gyro_y = gyro_y_raw / 16.4;
  gyro_z = gyro_z_raw / 16.4;

  printf("Gyroscope: X=%.2lf, Y=%.2lf, Z=%.2lf\n", gyro_x, gyro_y, gyro_z);

  float* gyro_values = (float*) malloc(3 * sizeof(float));
  gyro_values[0] = gyro_x;
  gyro_values[1] = gyro_y;
  gyro_values[2] = gyro_z;

  // x-axis: UP/DOWN
  if (gyro_x > 5) {
	  printf("TILT UP\n");
  }
  else if (gyro_x < -5) {
	  printf("TILT DOWN\n");
  }

  // y-axis: LEFT/RIGHT
  if (gyro_y > 5) {
          printf("TILT RIGHT\n");
  }
  else if (gyro_y < -5) {
          printf("TILT LEFT\n");
  }

  return gyro_values;
}

float* read_accel()
{
  int16_t acc_x_raw, acc_y_raw, acc_z_raw;
  acc_x_raw = (i2c_read_icm(0x0B) << 8) | i2c_read_icm(0x0C);
  acc_y_raw = (i2c_read_icm(0x0D) << 8) | i2c_read_icm(0x0E);
  acc_z_raw = (i2c_read_icm(0x0F) << 8) | i2c_read_icm(0x10);

  float acc_x, acc_y, acc_z;
  acc_x = (float) acc_x_raw / 2048;
  acc_y = (float) acc_y_raw / 2048;
  acc_z = (float) acc_z_raw / 2048;

  printf("Accelerometer: X=%.2lf, Y=%.2lf, Z=%.2lf\n", acc_x, acc_y, acc_z);

  float* accel_values = (float*) malloc(3 * sizeof(float));
  accel_values[0] = acc_x;
  accel_values[1] = acc_y;
  accel_values[2] = acc_z;

   // x-axis: LEFT/RIGHT
  if (acc_x > 0.05) {
          printf("MOVE LEFT\n");
  }
  else if (acc_x < -0.05) {
          printf("MOVE RIGHT\n");
  }

  // y-axis: FORWARD/BAKWARDS
  if (acc_y > 0.05) {
          printf("MOVE BACKWARD\n");
  }
  else if (acc_y < -0.05) {
          printf("MOVE FORWARDS\n");
  }

  return accel_values;
}
