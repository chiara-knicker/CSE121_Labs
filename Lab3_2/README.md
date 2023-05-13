# Lab 3.2

## File Descriptions

* `ble_mouse_main.c`
Code to use ESP32C3 as a mouse. This code moves the mouse left to right every five seconds.

* `esp_hidd_prf_api.h` & `esp_hidd_prf_api.c`
These files contain the the api of the HID profile
When you using the HID profile, you just need to include the esp_hidd_prf_api.h file and send the HID data using the function defined in the esp_hidd_prf_api.c file.

* `hid_dev.h` & `hid_dev.c`
These files define the HID spec related definitions

* `hidd_le_prf_int.h`
This header file includes some HID profile related definitions.

* `hid_device_le_prf.c`
This file is the HID profile definition file, it include the main function of the HID profile.
It mainly includes how to create HID service. If you send and receive HID data and convert the data to keyboard keys,
the mouse and consumer values are forwarded to the application.

## ble_mouse_main.c

```
static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param);
```
`hidd_event_callback` takes two arguments: an esp_hidd_cb_event_t and a pointer to an esp_hidd_cb_param_t. This function is the callback that will handle events related to the HID profile, such as when the BLE device is connected or disconnected. It will be called by the ESP-IDF framework.

```
#define HIDD_DEVICE_NAME            "ESP32C3_MOUSE"
```
This is the device name that will show up when looking for bluetooth devices.

```
static uint8_t hidd_service_uuid128[] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};
```
This defines an array hidd_service_uuid128 of 16 bytes (128 bits) that represents the UUID of the HID service. This UUID is a unique identifier that specifies the type of service offered by the BLE device.

```
static esp_ble_adv_data_t hidd_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x03c0,       //HID Generic,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(hidd_service_uuid128),
    .p_service_uuid = hidd_service_uuid128,
    .flag = 0x6,
};
```
This code is defining a static instance of the esp_ble_adv_data_t struct, which is used to configure the advertising data for a Bluetooth Low Energy (BLE) device.

* `set_scan_rsp`: A boolean value indicating whether the advertising data should be sent as a response to a scan request from a central device. In this case, it's set to false, meaning that the device will send the advertising data without being prompted by a scan request.

* `include_name`: A boolean value indicating whether the device name should be included in the advertising data. In this case, it's set to true, meaning that the device name will be included.

* `include_txpower`: A boolean value indicating whether the transmit power level should be included in the advertising data. In this case, it's set to true, meaning that the transmit power level will be included.

* `min_interval`: The minimum advertising interval in units of 1.25 milliseconds. In this case, it's set to 6, meaning that the device will advertise at a minimum interval of 7.5 milliseconds.

* `max_interval`: The maximum advertising interval in units of 1.25 milliseconds. In this case, it's set to 16, meaning that the device will advertise at a maximum interval of 20 milliseconds.

* `appearance`: The appearance value of the device. This is used to indicate the general category of the device, such as a mouse or a keyboard. In this case, it's set to 0x03c0, which corresponds to "HID Generic".

* `manufacturer_len and p_manufacturer_data`: The length and data of the manufacturer-specific data to be included in the advertising data. In this case, there is no manufacturer-specific data included, so the length is set to 0 and the data pointer is set to NULL.

* `service_data_len and p_service_data`: The length and data of the service-specific data to be included in the advertising data. In this case, there is no service-specific data included, so the length is set to 0 and the data pointer is set to NULL.

* `service_uuid_len and p_service_uuid`: The length and data of the service UUID to be included in the advertising data. In this case, the length is set to the size of the hidd_service_uuid128 array (which presumably contains the UUID for a HID service), and the data pointer is set to point to that array.

* `flag`: The advertising flags to be included in the advertising data. In this case, it's set to 0x6, which corresponds to "LE General Discoverable Mode" and "BR/EDR Not Supported".

```
static esp_ble_adv_params_t hidd_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x30,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};
```
This code is defining a structure called hidd_adv_params of type esp_ble_adv_params_t which contains the parameters for advertising as a BLE device. Here is a breakdown of each parameter:

* `adv_int_min`: This sets the minimum advertising interval between 2 advertisements. The value is 0x20, which is equivalent to 32 in decimal. The value is in units of 0.625 milliseconds, so the advertising interval is 32 x 0.625 = 20 milliseconds.

* `adv_int_max`: This sets the maximum advertising interval between 2 advertisements. The value is 0x30, which is equivalent to 48 in decimal. Again, the value is in units of 0.625 milliseconds, so the advertising interval is 48 x 0.625 = 30 milliseconds.

* `adv_type`: This specifies the type of advertising to be used. In this case, ADV_TYPE_IND is used, which is an indirect advertising type. This means that the device will send advertisements to a device that is listening, but will not wait for a response from the device it is advertising to.

* `own_addr_type`: This specifies the type of address to be used for advertising. In this case, BLE_ADDR_TYPE_PUBLIC is used, which means that the device will use a public address for advertising. Other types of addresses that can be used include static random and private resolvable addresses.

* `channel_map`: This parameter specifies which advertising channels to use. The value ADV_CHNL_ALL means that all three advertising channels will be used for advertising. There are three advertising channels in BLE: 37, 38, and 39.

* `adv_filter_policy`: This parameter specifies the advertising filter policy. In this case, ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY is used, which means that the device will allow connections from any device and will also allow scanning from any device. Other options include allowing connections from a whitelist of devices or only allowing connections from devices that have previously connected to the device.

```
static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
```
This is a callback function that will be invoked by the ESP-HIDD (Human Interface Device Device) library when certain events occur. The function takes two parameters: an event type (`esp_hidd_cb_event_t`) and a pointer to a parameter struct (`esp_hidd_cb_param_t`) containing additional information about the event.

The function uses a switch statement to handle different types of events.

* `ESP_HIDD_EVENT_REG_FINISH`: This event is triggered when the ESP-HIDD library finishes initializing. If the initialization was successful, the function sets the device name and advertising data.

* `ESP_BAT_EVENT_REG`: This event is not used in this code.

* `ESP_HIDD_EVENT_DEINIT_FINISH`: This event is triggered when the ESP-HIDD library finishes deinitializing. The code does nothing in response to this event.

* `ESP_HIDD_EVENT_BLE_CONNECT`: This event is triggered when a Bluetooth Low Energy (BLE) device connects to the HID device. The code stores the connection ID (`hid_conn_id`) for later use.

* `ESP_HIDD_EVENT_BLE_DISCONNECT`: This event is triggered when a BLE device disconnects from the HID device. The code sets a flag (`sec_conn`) to false and restarts advertising.

* `ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT`: This event is triggered when a vendor-specific report is received over BLE. The code logs the data to the console.

* `ESP_HIDD_EVENT_BLE_LED_REPORT_WRITE_EVT`: This event is triggered when an LED report is received over BLE. The code logs the data to the console.

Overall, the `hidd_event_callback` function serves as a way for the application to respond to various events that occur during HID operation. It sets the device name and advertising data when the HID device is initialized, and it responds to various events such as BLE connections and disconnections, as well as receiving data from BLE devices.

```
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
```
This defines the GAP (Generic Access Profile) event handler function that handles events related to Bluetooth Low Energy (BLE) connections, such as advertising and security. The function is registered with the BLE stack and will be called whenever an event occurs.

The function uses a switch statement to handle different types of events:

* `ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT`: This event is triggered when the advertising data has been set. The function starts advertising with the configured parameters (`hidd_adv_params`) using the `esp_ble_gap_start_advertising()` function.

* `ESP_GAP_BLE_SEC_REQ_EVT`: This event is triggered when a security request is received. The function logs the BD_ADDR (Bluetooth device address) of the requesting device using `ESP_LOGD()`, and then sends a security response to accept the request using `esp_ble_gap_security_rsp()`.

* `ESP_GAP_BLE_AUTH_CMPL_EVT`: This event is triggered when an authentication procedure has completed. The function logs the BD_ADDR and address type of the remote device, and whether the authentication was successful (`success` field). If authentication failed, it logs the reason (`fail_reason` field).


```
void hid_demo_task(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    while(1) {
        if (sec_conn) {
            ESP_LOGI(HID_DEMO_TAG, "Send mouse values");
	    // mouse values
	    uint8_t move_x = 50;
	    uint8_t move_y = 0;
	    uint8_t button = 0;
	    // send mouse values
	    esp_hidd_send_mouse_value(hid_conn_id, button, move_x, move_y);
	    vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}
```
This is the task for moving the mouse, which runs continuously in a loop. It waits for a secure connection to be established with the HID device (as indicated by the sec_conn variable being set to true), and then sends simulated mouse values to the device every 5 seconds using the esp_hidd_send_mouse_value function.

The mouse values are represented by three variables: move_x, move_y, and button. move_x and move_y indicate the amount of movement in the X and Y directions of the mouse, respectively, while button indicates which mouse button is being pressed. In this case, the mouse is set to move 50 pixels to the right (move_x is set to 50), with no movement in the Y direction (move_y is set to 0), and no mouse button being pressed (button is set to 0).

After sending the mouse values, the task delays for 5 seconds before sending the next set of mouse values.

```
void app_main(void)
```
This is the entry point of the application and is responsible for initializing and setting up the Bluetooth stack for the HID (Human Interface Device) profile.

1. `nvs_flash_init()`: Initializes the Non-Volatile Storage (NVS) partition. This partition is used to store configuration data that persists across power cycles.

2. If `nvs_flash_init()` returns an error code indicating that there are no free pages or a new version has been found, it erases the partition and tries to initialize it again.

3. `esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT)`: Releases the memory used by the Classic Bluetooth stack.

4. `esp_bt_controller_init(&bt_cfg)`: Initializes the Bluetooth controller with the default configuration.

5. `esp_bt_controller_enable(ESP_BT_MODE_BLE)`: Enables the Bluetooth controller in BLE mode.

6. `esp_bluedroid_init()`: Initializes the Bluedroid stack. Bluedroid is an open source Bluetooth stack developed by Broadcom and used in the Android operating system.

7. `esp_bluedroid_enable()`: Enables the Bluedroid stack.

8. `esp_hidd_profile_init()`: Initializes the HID profile.

9. `esp_ble_gap_register_callback(gap_event_handler)`: Registers the GAP (Generic Access Profile) callback function. The GAP profile is responsible for managing the device's connection to other Bluetooth devices.

10. `esp_hidd_register_callbacks(hidd_event_callback)`: Registers the HID callback function. This function is responsible for handling HID events, such as when a HID device is connected or disconnected.

11. `esp_ble_gap_set_security_param()`: Sets the security parameters for the BLE connection.

12. `xTaskCreate(&hid_demo_task, "hid_task", 2048, NULL, 5, NULL)`: Creates a new task named `hid_task` and assigns it the `hid_demo_task()` function to execute. This function is responsible for sending mouse values over the Bluetooth connection. The task has a stack size of 2048 bytes and a priority of 5.

## hid_dev.h

This header file defines various constants and types related to HID (Human Interface Device) functionality.

```
/* HID Report type */
#define HID_TYPE_INPUT       1
#define HID_TYPE_OUTPUT      2
#define HID_TYPE_FEATURE     3
```
`HID_TYPE_INPUT`, `HID_TYPE_OUTPUT`, and `HID_TYPE_FEATURE` represent the different types of HID reports that can be sent and received.

```
// HID Keyboard/Keypad Usage IDs (subset of the codes available in the USB HID Usage Tables spec)
#define HID_KEY_RESERVED       0    // No event inidicated
#define HID_KEY_A              4    // Keyboard a and A
#define HID_KEY_B              5    // Keyboard b and B
#define HID_KEY_C              6    // Keyboard c and C
...
typedef uint8_t keyboard_cmd_t;
```
```
// HID Mouse Usage IDs
#define HID_MOUSE_LEFT       253
#define HID_MOUSE_MIDDLE     254
#define HID_MOUSE_RIGHT      255
typedef uint8_t mouse_cmd_t;
```
```
// HID Consumer Usage IDs (subset of the codes available in the USB HID Usage Tables spec)
#define HID_CONSUMER_POWER          48  // Power
#define HID_CONSUMER_RESET          49  // Reset
#define HID_CONSUMER_SLEEP          50  // Sleep
...
typedef uint8_t consumer_cmd_t;
```
This defines various HID usage IDs for various keyboard keys, mouse buttons, and consumer control codes. These are used to indicate which key or button is being pressed, or which consumer control function is being activated. For example, the `HID_KEY_A`, `HID_KEY_B`, and `HID_KEY_C` constants are used to specify the letter keys on a keyboard, while the `HID_MOUSE_LEFT`, `HID_MOUSE_MIDDLE`, and `HID_MOUSE_RIGHT` constants are used to specify the left, middle, and right mouse buttons. Consumer control usage IDs are defined for common multimedia functions such as play, pause, and volume control.

It also defines various types of commands for different types of HID devices, such as `keyboard_cmd_t` for keyboards, `mouse_cmd_t` for mice, and `consumer_cmd_t` for consumer control devices.

```
// HID Consumer Control report bitmasks
#define HID_CC_RPT_NUMERIC_BITS         0xF0
#define HID_CC_RPT_CHANNEL_BITS         0xCF
#define HID_CC_RPT_VOLUME_BITS          0x3F
#define HID_CC_RPT_BUTTON_BITS          0xF0
#define HID_CC_RPT_SELECTION_BITS       0xCF

// Macros for the HID Consumer Control 2-byte report
#define HID_CC_RPT_SET_NUMERIC(s, x)    (s)[0] &= HID_CC_RPT_NUMERIC_BITS;   \
                                        (s)[0] = (x)
#define HID_CC_RPT_SET_CHANNEL(s, x)    (s)[0] &= HID_CC_RPT_CHANNEL_BITS;   \
                                        (s)[0] |= ((x) & 0x03) << 4
#define HID_CC_RPT_SET_VOLUME_UP(s)     (s)[0] &= HID_CC_RPT_VOLUME_BITS;    \
                                        (s)[0] |= 0x40
#define HID_CC_RPT_SET_VOLUME_DOWN(s)   (s)[0] &= HID_CC_RPT_VOLUME_BITS;    \
                                        (s)[0] |= 0x80
#define HID_CC_RPT_SET_BUTTON(s, x)     (s)[1] &= HID_CC_RPT_BUTTON_BITS;    \
                                        (s)[1] |= (x)
#define HID_CC_RPT_SET_SELECTION(s, x)  (s)[1] &= HID_CC_RPT_SELECTION_BITS; \
                                        (s)[1] |= ((x) & 0x03) << 4
```
This defines macros for manipulating HID Consumer Control 2-byte report, which is used for sending commands to a device that supports the HID (Human Interface Device) Consumer Control Usage Page.

The first set of lines defines bitmasks for each of the fields in the HID Consumer Control report, which are used to clear the relevant bits in the report before setting the new values.

The macros below are used to set the values of various fields in the HID Consumer Control report. They take a pointer to a buffer containing the 2-byte report, and modify the relevant bits in the buffer to set the desired value.

* `HID_CC_RPT_SET_NUMERIC`: Sets the numeric value of the report. The macro clears the upper 4 bits (0xF0) of the first byte of the report, and sets them to the specified value x.

* `HID_CC_RPT_SET_CHANNEL`: Sets the channel value of the report. The macro clears the upper 2 bits (0xCF) of the first byte of the report, and sets them to the specified value x shifted left by 4 bits.

* `HID_CC_RPT_SET_VOLUME_UP`: Sets the Volume Up button value of the report. The macro clears the upper 2 bits (0x3F) of the first byte of the report, and sets the 7th bit (0x40) to indicate that the Volume Up button is pressed.

* `HID_CC_RPT_SET_VOLUME_DOWN`: Sets the Volume Down button value of the report. The macro clears the upper 2 bits (0x3F) of the first byte of the report, and sets the 8th bit (0x80) to indicate that the Volume Down button is pressed.

* `HID_CC_RPT_SET_BUTTON`: Sets the value of a button in the report. The macro clears the upper 4 bits (0xF0) of the second byte of the report, and sets them to the specified value x.

* `HID_CC_RPT_SET_SELECTION`: Sets the value of a selection in the report. The macro clears the upper 2 bits (0xCF) of the second byte of the report, and sets them to the specified value x shifted left by 4 bits.

Setting various bits in the report indicate the state of different buttons or controls on the consumer control device.

```
// HID report mapping table
typedef struct
{
  uint16_t    handle;           // Handle of report characteristic
  uint16_t    cccdHandle;       // Handle of CCCD for report characteristic
  uint8_t     id;               // Report ID
  uint8_t     type;             // Report type
  uint8_t     mode;             // Protocol mode (report or boot)
} hid_report_map_t;
```
This is a structure definition for the HID report mapping table, which contains information about the characteristics of an HID report. Here's what each field means:

* `handle`: In Bluetooth, a "handle" is a unique identifier for a characteristic or descriptor.
* `cccdHandle`: Client Characteristic Configuration Descriptor (CCCD) is used to enable or disable notifications or indications for the characteristic.
* `id`: This is a unique identifier for the report, which is used to distinguish between multiple reports of the same type.
* `type`: This specifies whether the report is an input report, an output report, or a feature report.
* `mode`: This specifies whether the report is using the HID report protocol or the HID boot protocol.
```
// HID dev configuration structure
typedef struct
{
  uint32_t    idleTimeout;      // Idle timeout in milliseconds
  uint8_t     hidFlags;         // HID feature flags
} hid_dev_cfg_t;
```
This is a structure definition for the HID device configuration, which contains general configuration information for the HID device. Here's what each field means:

* `idleTimeout`: This specifies the amount of time that the device can remain inactive before it goes into an idle state.
* `hidFlags`: These flags are used to specify various HID features, such as whether the device supports wake-on-connect, remote wakeup, or the use of a boot protocol.

## hid_dev.c

This code defines functions that are used for implementing HID (Human Interface Device) profile in ESP32C3.

* `hid_dev_rpt_by_id(uint8_t id, uint8_t type)`: This is a helper function that searches the HID report mapping table (hid_dev_rpt_tbl) for a report with a given id and type, and returns a pointer to the corresponding hid_report_map_t structure. If no matching report is found, it returns NULL.

* `hid_dev_register_reports(uint8_t num_reports, hid_report_map_t *p_report)`: This function registers the HID report mapping table with the HID device. It takes two arguments: num_reports, the number of reports in the table, and p_report, a pointer to the table itself. It stores these values in global variables so that they can be accessed later by other functions.

* `hid_dev_send_report(esp_gatt_if_t gatts_if, uint16_t conn_id, uint8_t id, uint8_t type, uint8_t length, uint8_t *data)`: This function sends a HID report to the connected device over Bluetooth. It takes several arguments: gatts_if, the GATT interface to use for sending the report; conn_id, the connection ID of the device to send the report to; id and type, the ID and type of the report to send; length, the length of the report data; and data, a pointer to the report data. The function first calls hid_dev_rpt_by_id to find the report with the given id and type, and then uses the esp_ble_gatts_send_indicate function to send an indication of the report to the connected device.

* `hid_consumer_build_report(uint8_t *buffer, consumer_cmd_t cmd)`: This function builds an HID consumer control report based on a given command (cmd) and stores the result in a buffer. The buffer must be large enough to hold the report. The function uses a series of macros (HID_CC_RPT_SET_CHANNEL, HID_CC_RPT_SET_VOLUME_UP, etc.) to set the appropriate values in the report buffer based on the command. If the given buffer is NULL, the function logs an error and returns without modifying the buffer.
