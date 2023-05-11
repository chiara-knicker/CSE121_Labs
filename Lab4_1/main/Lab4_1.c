#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

#include "DFRobot_LCD.h"
#include "i2c_api.h"

//static const char *TAG = "lec2";

void app_main()
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
}
