# Lab 7.1

## Setup

[TODO: Explain wiring]

## Lab7_1.c

The ultrasonic sensor measures distance by emitting sound waves and calculating the time it takes for the waves to bounce back.

```
#define TRIGGER_GPIO 1
#define ECHO_GPIO 0
#define MAX_DISTANCE_CM 400
#define TIMEOUT_US (MAX_DISTANCE_CM * 58)
```
This defines which GPIO pins are used for sending the trigger signal and receiving the echo signal. It also defines the maximum distance that the ultrasonic sensor can measure (400cm) as wel as the timeout value, which is calculated by multiplying the maximum distance by 58, which is the conversion factor from microseconds to centimeters for the speed of sound (1/34,000cm/s * 2 * 10^-6 = 58).
```
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = 1ULL << TRIGGER_GPIO,
	};
	gpio_config(&io_conf);
```
This code sets up the trigger GPIO pin for the trigger signals:

It creates a gpio_config_t structure called io_conf and sets the intr_type (interrupt type) to disable interrupts, mode to output mode, and pin_bit_mask to set the trigger GPIO pin as an output.
It then calls gpio_config() to apply the configuration to the GPIO pins.
```
	io_conf.intr_type = GPIO_INTR_POSEDGE,
	io_conf.mode = GPIO_MODE_INPUT,
	io_conf.pin_bit_mask = 1ULL << ECHO_GPIO,
	gpio_config(&io_conf);
```
This code reconfigures the io_conf structure to set up the echo GPIO pin:

It sets the intr_type to positive edge interrupt and mode to input mode.
It updates the pin_bit_mask to set the echo GPIO pin as an input.
It calls gpio_config() again to apply the updated configuration.
```
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
```
This continuous loop performs distance measurements repeatedly.

Inside the loop, the code generates a trigger pulse to start the ultrasonic sensor measurement:
- It sets the trigger GPIO pin to 0 (low) for a short delay
- It sets the trigger GPIO pin to 1 (high) for a specific duration (10 microseconds in this case) to trigger the ultrasonic sensor
- It sets the trigger GPIO pin back to 0 to end the trigger pulse

After triggering the sensor, the code waits for the echo signal to start:
- It records the current time using esp_timer_get_time()
- It enters a loop that checks if the echo GPIO pin is low (0), and it also checks if the elapsed time since the start is within the timeout limit (TIMEOUT_US)
- The loop continues until either the echo signal starts (GPIO pin goes high) or the timeout limit is reached

Once the echo signal starts, the code measures the time it takes for the echo signal to end:
- It updates the start variable with the current time
- It enters a loop that checks if the echo GPIO pin is high (1), and it also checks if the elapsed time since the start is within the timeout limit
- The loop continues until either the echo signal ends (GPIO pin goes low) or the timeout limit is reached

The code then calculates the distance based on the time elapsed:
- It subtracts the start time from the current time using esp_timer_get_time()
- It divides the elapsed time by 58 (microseconds per centimeter) to convert it to distance in centimeters
- The calculated distance is stored in the distance variable

Finally, the code prints the measured distance in centimeters using printf(). It waits for 1 second using vTaskDelay() before starting the next distance measurement.

## Resources

Ultrasonic Sensor <br>
https://lastminuteengineers.com/arduino-sr04-ultrasonic-sensor-tutorial/?utm_content=cmp-true
