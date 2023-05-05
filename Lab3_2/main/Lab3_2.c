#include <stdio.h>
#include <string.h>
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#define DEVICE_NAME "ESP32-C3 Mouse"

/* Mouse Report Descriptor */
static uint8_t mouse_report_descriptor[] = {
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x02,       /* Usage (Mouse) */
    0xA1, 0x01,       /* Collection (Application) */
    0x09, 0x01,       /* Usage (Pointer) */
    0xA1, 0x00,       /* Collection (Physical) */
    0x05, 0x09,       /* Usage Page (Buttons) */
    0x19, 0x01,       /* Usage Minimum (01) */
    0x29, 0x01,       /* Usage Maximum (01) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x25, 0x01,       /* Logical Maximum (1) */
    0x95, 0x01,       /* Report Count (1) */
    0x75, 0x01,       /* Report Size (1) */
    0x81, 0x02,       /* Input (Data, Variable, Absolute) */
    0x95, 0x01,       /* Report Count (1) */
    0x75, 0x05,       /* Report Size (5) */
    0x81, 0x01,       /* Input (Constant) */
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x30,       /* Usage (X) */
    0x09, 0x31,       /* Usage (Y) */
    0x15, 0x81,       /* Logical Minimum (-127) */
    0x25, 0x7F,       /* Logical Maximum (127) */
    0x75, 0x08,       /* Report Size (8) */
    0x95, 0x02,       /* Report Count (2) */
    0x81, 0x06,       /* Input (Data, Variable, Relative) */
    0xC0,             /* End Collection */
    0xC0              /* End Collection */
};

/* Advertisement data */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x03c2, // mouse
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = 0x6, // BLE-only device, does not support BR/EDR feature
    // .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

/* BLE GATT server configuration */
/*
static esp_ble_gatts_cb_t gatts_cb = {};

static esp_ble_gatts_attr_db_t gatts_attr_db[] = {
    {
        .attr_control = {
            .auto_rsp = ESP_GATT_AUTO_RSP,
        },
        .att_desc = {
		.uuid = {
			.len = ESP_UUID_LEN_16,
			.uuid = {.uuid16 = ESP_GATT_UUID_HID_REPORT},
		},
		.perm = ESP_GATT_PERM_READ,
        	.max_len = 50,
        	.length = sizeof(mouse_report_descriptor),
        	.p_value = mouse_report_descriptor,
    	},
    },
};

static esp_ble_gatts_service_t gatts_service = {
	.uuid = {
		.len = ESP_UUID_LEN_16,
		.uuid = {.uuid16 = ESP_GATT_UUID_HIDS},
	},
	.inst_id = 0,
	.is_primary = true,
	.num_handle = 1,
	.included_service = NULL,
	.handle = NULL,
	.end_handle = NULL,
	.characteristic = gatts_attr_db,
};*/

/* GATT server event handler */
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            // Register app ID
            esp_ble_gatts_app_register(PROFILE_APP_ID);
            break;
        case ESP_GATTS_CONNECT_EVT:
            // Store connection ID and set connection flag
            conn_id = param->connect.conn_id;
            connected = true;
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            // Clear connection flag
            connected = false;
            break;
        default:
            break;
    }
}

/* Initialize Bluetooth */
void bt_init() {
	esp_err_t ret;

	/* Initialize BT stack */
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	bt_cfg.mode = ESP_BT_MODE_BTDM;
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret != ESP_OK) {
    		printf("Error initializing BT controller: %d\n", ret);
    		return;
	}

	/* Enable BT */
	ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (ret != ESP_OK) {
    		printf("Error enabling BT controller: %d\n", ret);
    	return;
	}

	/* Initialize BLE stack */
	ret = esp_bluedroid_init();
	if (ret != ESP_OK) {
    		printf("Error initializing BLE stack: %d\n", ret);
    		return;
	}

	/* Enable BLE */
	ret = esp_bluedroid_enable();
	if (ret != ESP_OK) {
    		printf("Error enabling BLE: %d\n", ret);
    		return;
	}

	// Register GATT server callback
	esp_ble_gatts_register_callback(gatts_event_handler);

	// Set device name
	esp_ble_gap_set_device_name(DEVICE_NAME);

	// Set advertising data
	esp_ble_gap_config_adv_data(&adv_data);

	/* Register HID service */
	/*
	esp_ble_gatts_create_service(ESP_GATT_SERVICE_TYPE_PRIMARY, &gatts_service.uuid, 0x50);
	esp_ble_gatts_start_service(1);*/

	// Create HID service
    	esp_ble_gatts_create_service(gatts_if, &hid_service_uuid, GATT_NUM_HANDLES);

    	// Add HID report characteristic
    	esp_ble_gatts_add_char(gatts_if, hid_service_handle_table[HID_SERVICE], &hid_report_char_uuid, HID_REPORT_DESCR_SIZE, ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY, &hid_report_descr_val, NULL);

    	// Start advertising
    	esp_ble_gap_start_advertising(&adv_params);
}

/* HID mouse report */
typedef struct {
    uint8_t buttons;  // Button state (bits 0-2)
    int8_t x;         // X movement
    int8_t y;         // Y movement
} __attribute__((packed)) mouse_report_t;

static mouse_report_t mouse_report = { 0, 0, 0 };

/* Send a mouse HID report */
static void send_mouse_report(void) {
    esp_err_t ret;
    esp_ble_gatts_send_indicate(gatts_if, conn_id, char_handle, sizeof(mouse_report), (uint8_t *)&mouse_report, false);
}

int main() {
	/* Initialize Bluetooth */
	bt_init();
	/* Set device name */
	//esp_ble_gap_set_device_name(DEVICE_NAME);

	/* Set advertisement data */
	//esp_ble_gap_config_adv_data(&adv_data);

	/* Start advertising */
	//esp_ble_gap_start_advertising(NULL);

	/* Loop forever */
	while (1) {
		// Move the mouse right by 10 units and press the left button
        	mouse_report.buttons = 1;  // Left button pressed
        	mouse_report.x = 10;      // Move right by 10 units
        	mouse_report.y = 0;       // No vertical movement
        	send_mouse_report();
    		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

