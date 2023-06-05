# Lab 4.2

This lab is based on the DFRobot_LCD GitHub repository.

https://github.com/DFRobotdl/DFRobot_LCD

## File description

* `Lab4_2.c`
Code for reading temperature and displaying the values on the LCD.

* `i2c_api.h` & `i2c_api.c`
Files containing api for i2c communication and reading the icm sensor (accelerometer and gyroscope) and shct3 sensor (temperature and humidity).

* `DFRobot_LCD.h` & `DFRobot_LCD.c`
DFRobot_LCD repository code translated to C and compatible with the ESP32C3. Implements functionality to use LCD display.

## Lab4_2.c

First, I am initializing the the LCD with the init() function from the DFRobot_LCD library. Then I am calling the shtc3_task() function, which continuously reads the temperature sensor and sends the temperature measurements to the LCD to display them.

It is important to note that I am not calling the i2c_master_init() function in the while loop of the shtc3_task() function because it is already called inside the init() function for the LCD display.
