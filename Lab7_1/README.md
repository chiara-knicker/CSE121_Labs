# Lab 7.1

## Setup

[TODO: Explain wiring]

## Lab7_1.c

TODO:

```
#define TRIGGER_GPIO 1
#define ECHO_GPIO 0
#define MAX_DISTANCE_CM 400
#define TIMEOUT_US (MAX_DISTANCE_CM * 58)

void app_main()
{
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = 1ULL << TRIGGER_GPIO,
	};
	gpio_config(&io_conf);

	io_conf.intr_type = GPIO_INTR_POSEDGE,
	io_conf.mode = GPIO_MODE_INPUT,
	io_conf.pin_bit_mask = 1ULL << ECHO_GPIO,
	gpio_config(&io_conf);

	while (true)
	{
		// Send trigger pulse
		gpio_set_level(TRIGGER_GPIO, 0);
		esp_rom_delay_us(2);
		gpio_set_level(TRIGGER_GPIO, 1);
		esp_rom_delay_us(10);
		gpio_set_level(TRIGGER_GPIO, 0);

		// Wait for echo start
		uint32_t start = esp_timer_get_time();
		while (gpio_get_level(ECHO_GPIO) == 0 && esp_timer_get_time() - start < TIMEOUT_US);

		// Measure echo time
		start = esp_timer_get_time();
		while (gpio_get_level(ECHO_GPIO) == 1 && esp_timer_get_time() - start < TIMEOUT_US);

		// Calculate distance in cm
		uint32_t distance = (esp_timer_get_time() - start) / 58;
		printf("Distance: %d cm\n", (int) distance);

		// Wait for 1 second
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
```
