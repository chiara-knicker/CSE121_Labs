# Lab 2.2
## Importing Libraries
```
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
```
These libraries provide functions and data types to interact with the FreeRTOS operating system, the I2C hardware and the ESP32 logging system.

## I2C Configuration
```
#define I2C_MASTER_SCL_IO 8 /*!< GPIO number for I2C master SCL */
#define I2C_MASTER_SDA_IO 10 /*!< GPIO number for I2C master SDA */
```
The GPIO numbers for SCL and SDA can be found on the github page for the ESP32. They are used for I2C clock and data lines, respectively.
```
#define I2C_MASTER_NUM I2C_NUM_0
```
This is the I2C bus number, which can be 0 or 1 on the ESP32. It is set to 0 so the sensor is connected to the master I2C bus.
```
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
```
Setting these to 0 means that the RX and TX buffer on the I2C driver are enabled. This means that the I2C driver will use internal buffers for both read and write operations. These buffers are used to store data as it is transmitted and received over the I2C bus. This is relevant to clock stretching because the I2C protocol allows the slave device to hold the clock line low to pause the master during data transmission.
```
#define I2C_MASTER_FREQ_HZ 100000
```
This is the clock frequency of the I2C bus, which is set to 100kHz.
```
#define SHTC3_SENSOR_ADDR 0x70 /*!< SHTC3 12C address */
```
This is the I2C address of the SHTC3 sensor, which can be found on the github page for the ESP32.

## Commands for communication
```
#define SHTC3_CMD_READ_ID OxEFCS8
#define SHTC3_CMD_MEASURE 0x7CA2
```
These commands are sent to the sensor to request the sensor ID and to initiate temperature and humidity measurements.

0xEFCS8 tells sensor to read out the ID register and can be used to verify the presence of the sensor and proper communication. It can be found in the datasheet (5.9).

0x7CA2 enables clock stretching, puts the sensor in normal mode and tells it to read the temperature first. The datasheet (5.3) contains a table that summarizes the available measurement commands.

## I2C Initialization
```
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
```
This initializes the i2c_config_t structure using the I2C configuration defined earlier.
```
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to i2c_param_config %d", err);
        return err;
    }
```
i2c_param_config() sets the I2C configuration for the specified I2C port. 
```
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE,0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to i2c_driver_install %d", err);
        return err;
    }

```
i2c_driver_install() installs the I2C driver for a specific I2C bus. The 0 is the clock speed of the I2C bus, which is set to the default value of 100kHz. (Should have used I2C_MASTER_FREQ_HZ ???)

## Read Temperature and Humidity
```
static esp_err_t shtc3_read(uint16_t command, uint8_t *data, size_t size)
```
shtc3_read() reads the temperature and humidity data from the SHTC3 sensor. It takes 3 parameters:

command: command code to be sent to sensor
data: pointer to an array that will store received data
size: size of data to be received

```
i2c_cmd_handle_t cmd = i2c_cmd_link_create();
```
This creates an I2C command handle used to queue the I2C commands that will be sent to the sensor.
```
i2c_master_start(cmd);
```
This sends the start condition.
```
i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR <<1) | I2C_MASTER_WRITE, true);
i2c_master_write_byte(cmd, command >> 8, true);
i2c_master_write_byte(cmd, command & 0xFF, true);
```
These send the sensor's address and write bit, the command code and its arguments.

(SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE: creates a byte that contains both the sensor address (shifted left by 1 bit) and the write bit (set to 0). The | operator is a bitwise OR operator that combines the two values. The 8th bit in the address byte is used to indicate whether the master device wants to write to or read from the slave device.

command >> 8 is shifting the command variable 8 bits to the right. This is equivalent to dividing the value by 256 (2^8), which effectively extracts the upper 8 bits of the 16-bit command value.

command & 0xFF is using the bitwise AND operator to mask the lower 8 bits of the command variable, which extracts the lower 8 bits of the 16-bit command value. The 0xFF is a binary number that has all its bits set to 1 in the lower 8 bits and all its bits set to 0 in the upper 8 bits, which effectively masks out the upper 8 bits of the command value.
```
i2c_master_stop(cmd);
```
This sends the stop condition.
```
err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
```
i2c_master_cmd_begin() is used to execute an I2C transaction using the given I2C command descriptor cmd. This function sends the command to the I2C slave device and waits for its response.

pdMS_TO_TICKS(1000) function call converts 1000 milliseconds to the equivalent FreeRTOS ticks, which is used as the timeout parameter for the transaction. This means that the function will wait for a maximum of 1000 milliseconds (1 second) for the I2C transaction to complete before timing out.
```
vTaskDelay(pdMS_TO_TICKS(20));
```
After sending the command to the sensor, the function waits for 20 milliseconds to allow the sensor to complete the measurement.
```
i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
```
This sends the sensor's address and the read bit.
```
i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
```
This reads the data from the sensor and store it in the data array.

## Calculate Temperature and Humidity
calculate_humidity, calculate_temp and calculate_temp_f take the raw measurement and return the actual humidity and temperature value (Celsius and Fahrenheit). The formulas can be found on the sensor datasheet (5.11).

## shtc3_task
This function is the main task that reads sensor data and calculates the actual temperature and humidity values using the previous functions.
```
uint8_t data[6] = {0,};
uint16_t raw_humidity=0;
uint16_t raw_temp=0;
```
This declares some variables to store the sensor data. 
```
esp_err_t err = shtc3_read(SHTC3_CMD_MEASURE, data, 6);
```
This calls the shtc3_read function with the SHTC3_CMD_MEASURE command to read the temperature and humidity data from the sensor. 
```
if(err == ESP_OK){
      raw_humidity = (data[3] << 8) | data[4];
      raw_temp = (data[0] << 8) | data[1];
      float humidity = calculate_humidity(raw_humidity);
      float temp = calculate_temp(raw_temp);
      float temp_f = calculate_temp_f(raw_temp);
      ESP_LOGI(TAG, "Temperature is %.2fC (or %.2fF) with a %.2f %%  humidity", temp, temp_f, humidity);
  } else {
      ESP_LOGI(TAG, "Failed to read data from SHTC3 sensor %d", err);
  }
```
If the reading is successful, the function extracts the raw humidity and temperature values from the received data and calculates the actual temperature and humidity values using the calculate_humidity, calculate_temp, and calculate_temp_f functions. 

The sensor sends the temperature first and then the humidity data, as we specified with the measurement command. The sensor sends a total of 6 bytes, the first 3 bytes are for temperature and the last 3 bytes are for humidity. The first two bytes of each 3 byte block are the measurements and the third byte is a checksum that we won't need. This information can be found in the datasheet (5.4).

To parse the temperature and humidity data from the raw bytes, we first need to combine the two bytes for each measurement into a single 16-bit integer. We can do this using bit shifting and bitwise OR operations. data[0] and data[3] contain the high byte of the temperature and humidity data respectively, and data[1] and data[4] contain the low byte. We shift data[0] and data [3] left by 8 bits (the equivalent of multiplying by 256) to make room for the low byte, and then use a bitwise OR operation to combine the two bytes into a single 16-bit integer.

Finally, the function prints the calculated values using the ESP_LOGI function, which logs a message to the console with the specified tag, message, and arguments.
```
vTaskDelay(pdMS_TO_TICKS(2000));
```
This makes the function waits for 2 seconds before starting the loop again. This loop will continue indefinitely, continuously reading the sensor data and printing the actual temperature and humidity values to the console.
