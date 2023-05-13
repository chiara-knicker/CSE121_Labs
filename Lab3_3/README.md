# Lab 3.3

## File descriptions

* `ble_mouse_main.c`
Code to use ESP32C3 as a mouse. This code moves the mouse left to right every five seconds.

* `gyro_accel_api.h` & `gyro_accel_api.c`
These files contain the api for using i2c and reading the gyroscope and accelerometer. This is the same code as from previous labs but cleaned up and split into a header and source file.

* `esp_hidd_prf_api.h` & `esp_hidd_prf_api.c`
These files contain the api of the HID profile
When you using the HID profile, you just need to include the esp_hidd_prf_api.h file and send the HID data using the function defined in the esp_hidd_prf_api.c file.

* `hid_dev.h` & `hid_dev.c`
These files define the HID spec related definitions

* `hidd_le_prf_int.h`
This header file includes some HID profile related definitions.

* `hid_device_le_prf.c`
This file is the HID profile definition file, it include the main function of the HID profile.
It mainly includes how to create HID service. If you send and receive HID data and convert the data to keyboard keys,
the mouse and consumer values are forwarded to the application.

## Setup

```
cd Lab3_2
```
```
idf.py menuconfig
```
This opens a configuration menu. Select Bluetooth and enable it. Building should now have more steps.

## ble_mouse_main.c

```
void hid_demo_task(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    while(1) {
	      float* accel_values = read_accel();

        vTaskDelay(pdMS_TO_TICKS(100));
        if (sec_conn) {
            ESP_LOGI(HID_DEMO_TAG, "Send mouse values");
	          // mouse values
	          int8_t move_x = 10 * accel_values[0];
	          int8_t move_y = 10 * accel_values[1];
	          int8_t button = 0;
	          // send mouse values
	          esp_hidd_send_mouse_value(hid_conn_id, button, move_x, move_y);
	          vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}
```
This is the task for moving the mouse according to the accelerometer values, which runs continuously in a loop. It waits for a secure connection to be established with the HID device (as indicated by the `sec_conn` variable being set to `true`), and then sends the accelerometer values to the device using the `esp_hidd_send_mouse_value` function.

The function first calls the `read_accel()` function to obtain the accelerometer values, which are stored in `accel_values`. `accel_values` will contain the x, y and z values measured by the accelerometer.

The mouse values are represented by three variables: `move_x`, `move_y`, and `button`. `move_x` and `move_y` indicate the amount of movement in the X and Y directions of the mouse, respectively, while `button` indicates which mouse button is being pressed. In this case, the mouse is set to move 10 times the x value of the accelerometer pixels to the right, 10 times the y value of the accelerometer pixels to the left, and no mouse button being pressed (`button` is set to 0).

After sending the mouse values, the task delays for 500 milliseconds before sending the next set of mouse values. 

```
void app_main(void)
{
    ...

    i2c_master_init();
    icm_init();
    acc_conf();
    gyr_conf();

    xTaskCreate(&hid_demo_task, "hid_task", 2048, NULL, 5, NULL);
}
```
This first initializes the I2C communication by calling the `i2c_master_init()`function. Then it initializes the icm sensor and configures the accelerometer and gyroscope. Finally, it creates a new task named hid_task and assigns it the hid_demo_task() function to execute. This function is responsible for sending the mouse values over the Bluetooth connection. The task has a stack size of 2048 bytes and a priority of 5.
