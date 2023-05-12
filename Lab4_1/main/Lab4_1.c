#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>
#include "esp_log.h"

#include "DFRobot_LCD.h"
#include "i2c_api.h"

//static const char *TAG = "lec2";

void shtc3_task(){
    while(1){

        uint8_t data[6] = {0,};
        uint16_t raw_humidity=0;
	uint16_t raw_temp=0;

	i2c_write_shtc3(SHTC3_CMD_MEASURE);
	i2c_read_shtc3(data, 6);

        raw_humidity = (data[3] << 8) | data[4];
	raw_temp = (data[0] << 8) | data[1];
        float humidity = calculate_humidity(raw_humidity);
	float temp = calculate_temp(raw_temp);
	float temp_f = calculate_temp_f(raw_temp);
        
	printf("Temperature is %.2fC (or %.2fF) with a %.2f %%  humidity\n", temp, temp_f, humidity);
        
	setRGB(0, 255, 0);
	setCursor(0,0);
	print("Hello CSE121!");
	setCursor(0,1);
	print("Chiara");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void){
    /*esp_err_t err = i2c_master_init();
    if(err != ESP_OK) {
        printf("Failed to initialize I2C master\n");
        return;
    }*/

    init();
    shtc3_task();
}   
