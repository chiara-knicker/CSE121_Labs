# Lab 4.1

This lab is based on the DFRobot_LCD GitHub repository.

https://github.com/DFRobotdl/DFRobot_LCD

## File description

* `Lab4_1.c`
Code for implementing printing "Hello CSE121! Chiara" on the LCD.

* `i2c_api.h` & `i2c_api.c`
Files containing api for i2c communication and reading the icm sensor (accelerometer and gyroscope) and shct3 sensor (temperature and humidity).

* `DFRobot_LCD.h` & `DFRobot_LCD.c`
DFRobot_LCD repository code translated to C and compatible with the ESP32C3. Implements functionality to use LCD display.

## DFRobot_LCD.h & DFRobot_LCD.c

I replaced all occurrences of Wire with C code compatible with the ESPC3. For this, I looked at the Wire library to see what each function was doing so I could replace it correctly.

* Wire.begin: The master begin function basically initializes the I2C connection. Therfore, I replaced it with the i2c_master_init() function.
* Wire.beginTransmission: This function sets the address and txLength = 0, which is the length of the txBuffer which will ultimately contain all the data to be sent to the specified address. Therefore, I replaced it with 
```
i2c_cmd_handle_t cmd = i2c_cmd_link_create();
i2c_master_start(cmd);
i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
```
* Wire.write: This function adds data to the txBuffer and increases txLength. Therefore, I replaced it with the i2c_write_byte() function.
* Wire.endTransmission: This function sends the txBuffer contents to the specified address. Therefore, I replaced it with
```
i2c_master_stop(cmd);
esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
i2c_cmd_link_delete(cmd);
```

To make the code compatible with C and the ESP32C3, I also made some other adjustments:

* Removed <Arduino.h>, <Wire.h> and "Print.h" libraries
* `delay()` and `delayMicroseconds()`: replaced with vTaskDelay(pdMS_TO_TICKS(time)).
* Removed DFRobot_LCD class: 
*   declared and defined the `lcd_Addr`, `RGB_address`, `_cols` and `_rows` at beginning of source file
*   adjusted all functions accordingly (removed class name in front of them)
* Defined functions defined in header file in source file instead
* Declared variables at beginning of source file instead of in header file
* `using Print::write`: imlemented own print function that utilizes the `write()` function
* `virtual size_t write(uint8_t)`: removed virtual to make it a regular function
* `void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS)`: included LCD_5x8DOTS in `begin()` function call

I also changed the write function to return void instead of size_t since I do not need it to return anything.

## Sending commands to LCD

* command()
The command() function takes an 8 bit integer value as input. This value is the command that should be sent to the LCD. It is stored in a buffer together with the value 0x80. The binary representation of 0x80 is 10000000. This is the control byte that has to be sent before every data byte (Datasheet p.6). The function then calls the send() function to send the control byte and data byte (input value).

* send()
The send() function takes the data and data length as input. It then loops over the data buffer to write each byte to i2c.

* begin()
The begin() function initializes the LCD display with the specified settings. It first sends the Function Set command, which sets the number of display lines and the display font type. Then it sends the Entry Mode Set command to turn the display on with no cursor and blinking default. Finally, it initialized the backlight and sets it's color to white. The appropriate commands are constructed using binary operations. The command formats can be found in the datasheet on page 9.

## Writing to LCD

* write()
The write function is similar to the send() function, but it is used to write characters to the LCD to display them. Since it isn't sending a command to the LCD, it sends a different control byte before the data byte. The print() function repeatedly calls the write function when iterating over the input String to send one character at a time that should be displayed on the LCD.

## Sending data to RGB backlight

* setReg()
The setReg() function is similar to the send() function, but it is used to write to the RGB light. It is used to send the RGB values to the backlight to change its color. The address specifies the color register (R, G, B) and the data is the color value to set it to. Writing to the backlight does not require a control byte.
