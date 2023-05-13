#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

#include "DFRobot_LCD.h"
#include "i2c_api.h"

//static const char *TAG = "lec2";

void app_main(void)
{
    init();
    setRGB(0, 255, 0);
    setCursor(0,0);
    print("Hello CSE121!");
    setCursor(0,1);
    print("Chiara");
} 
