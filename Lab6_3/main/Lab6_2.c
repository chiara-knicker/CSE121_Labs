#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_system.h"

#define ADC_CHANNEL ADC_CHANNEL_0

void app_main()
{
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

	while(1) {
		int val = adc1_get_raw(ADC_CHANNEL);
		printf("Raw: %d\n", val);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
