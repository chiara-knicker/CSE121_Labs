#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

#include "gyro_accel_api.h"

static const char *TAG = "lec2";

//#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number
//#define I2C_MASTER_SCL_IO 8 // GPIO number for I2C clock
//#define I2C_MASTER_SDA_IO 10 // GPIO number for I2C data
//#define I2C_MASTER_FREQ_HZ 100000 // I2C master clock frequency

//#define ICM_ADDR 0x68 // I2C address of the ICM-42670-P sensor

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

void i2c_write(uint8_t reg, uint8_t data) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (ICM_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true); // ACCEL_CONFIG0 register
  i2c_master_write_byte(cmd, data, true); // Set full-scale range to +/-16g
  i2c_master_stop(cmd);
  esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(2000));
  i2c_cmd_link_delete(cmd);

  if (err != ESP_OK) {
    printf("Failed to write to I2C\n");
    printf("Error: %d\n", err);
  }
}

void icm_init() {
  // PWR_MGMT0 register, gyro and eccel in LN mode and RC oscillator on
  i2c_write(0x1F, 0b00011111);
}

void acc_conf() {
  // ACCEL_CONFIG0 register, set full-scale range to +/-16g 
  i2c_write(0x21, 0b00011111);
}

void gyr_conf() {
  // GYRO_CONFIG0 register, set full-scale range to +/-2000dps
  i2c_write(0x20, 0b00001001);
}


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

float*  read_gyro() {
  int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
  gyro_x_raw = (i2c_read(0x11) << 8) | i2c_read(0x12);
  gyro_y_raw = (i2c_read(0x13) << 8) | i2c_read(0x14);
  gyro_z_raw = (i2c_read(0x15) << 8) | i2c_read(0x16);

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

float* read_accel() {
  int16_t acc_x_raw, acc_y_raw, acc_z_raw;
  acc_x_raw = (i2c_read(0x0B) << 8) | i2c_read(0x0C);
  acc_y_raw = (i2c_read(0x0D) << 8) | i2c_read(0x0E);
  acc_z_raw = (i2c_read(0x0F) << 8) | i2c_read(0x10);

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


/*void app_main()
{
  i2c_master_init();
  icm_init();
  acc_conf();
  gyr_conf();

  while (true)
  {
    read_gyro();
    read_accel();
    vTaskDelay(pdMS_TO_TICKS(500)); // wait 100 milliseconds before reading again
  }
}*/
