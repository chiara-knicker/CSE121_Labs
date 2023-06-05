# Lab 4.2

For this lab I simply created a header and source file with all the necessary functions to communicate with the sensors on the ESP32 via I2C. I used the functions from previous labs. It is important to note that I am not calling the i2c_master_init() function in the while loop of the shtc3_task() function because it is already called inside the init() function for the LCD display.
