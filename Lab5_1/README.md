# Lab 5.1

This lab is using the station example from the esp-idf GitHub repository.

https://github.com/espressif/esp-idf/tree/17451f1fb3d6485af5113d96b6836a7ce60efee8/examples/wifi/getting_started/station

## File descriptions

* ```local_server_cert.perm``` certificate needed to send HTTPS request using cacert buf, global ca store or to local server
* ```server_root_cert.perm``` certificate needed to send HTTPS request using cacert buf, global ca store or to local server
* ```station_example_main.c``` contains functionality to send requests

## station_example_main.c

### Connecting to Wifi
```
#define WEB_SERVER "www.wttr.in"
#define WEB_PORT "443"
// HTTPS
#define WEB_URL "https://www.wttr.in/Santa+Cruz?format=%l:+%c+%t"
#define SERVER_URL_MAX_SZ 256
// HTTP
#define WEB_PATH "/Santa+Cruz?format=%l:+%c+%t/"

/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD (86400000000ULL)

#define CONNECTION 2
```
This defines the constants that are needed for the HTTP(S) requests. These can be adjusted based on the desired destination of the request and what protocol should be used (HTTP or HTTPS). Right now, they are set to send a HTTPS request to www.wttr.in to get the weather in Santa Cruz.

The CONNECTION constant is used to specify which wifi to connect to since I was working on this lab at different places and wanted an easy way to switch between wifi networks.

```
#if CONNECTION == 0
    // Configured wifi
    #define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
    #define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#elif CONNECTION == 1

...

#elif CONNECTION == 3
    // Eduroam
    #define EXAMPLE_ESP_WIFI_SSID      "eduroam"
    #define EXAMPLE_ESP_WIFI_USER      "user email"
    #define EXAMPLE_ESP_WIFI_PASS      "password"
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY
#endif
```
This specifies the Wifi SSID, Wifi password and maximum number of retries for several wifi networks. The first one uses the values specified in menuconfig (idf.py menuconfig). The following ones are different wifi networks I connected to when working on the lab. The last one is to connect to Eduroam. Thisadditionally specifies the user since Eduroam has some extra authorization that regular wifi networks like the typical home wifi don't have.

```
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
...
}
```
The event_handler function is an event callback function that handles Wi-Fi and IP events. The purpose of this function is to handle Wi-Fi and IP events related to Wi-Fi station (STA) mode. It manages the Wi-Fi connection process, handles disconnections and reconnection retries, and tracks the successful acquisition of an IP address.

This function handles these events:
- WIFI_EVENT_STA_START: When this event occurs, it triggers a call to esp_wifi_connect(), which initiates the Wi-Fi connection process.
- WIFI_EVENT_STA_DISCONNECTED: If the Wi-Fi station disconnects, it checks whether the maximum number of connection retries (EXAMPLE_ESP_MAXIMUM_RETRY) has been reached. If not, it attempts to reconnect by calling esp_wifi_connect() again, increments the s_retry_num counter, and logs a retry message. If the maximum retries have been reached, it sets a bit in the s_wifi_event_group event group (WIFI_FAIL_BIT) to indicate a connection failure.
- IP_EVENT_STA_GOT_IP: This event indicates that the station has obtained an IP address. It retrieves the IP address information from the event_data parameter, logs the obtained IP address, resets the s_retry_num counter, and sets a bit in the s_wifi_event_group event group (WIFI_CONNECTED_BIT) to indicate successful connection.

```
void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ...
    
    // Normal wifi
    wifi_config_t wifi_config = {
	.sta = {
		.ssid = EXAMPLE_ESP_WIFI_SSID,
		.password = EXAMPLE_ESP_WIFI_PASS,
		.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
           	.sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            	.sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    
    // Eduroam 
    if (CONNECTION == 3) {
    	// Eduroam
	    wifi_config_t wifi_config = {
            .sta = {
            	.ssid = EXAMPLE_ESP_WIFI_SSID,
            	.password = "exampleEmail:examplePassword",
            	.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE,
	    	.pmf_cfg = {
                    .capable = true,
                    .required = false,
            	},
            },
    	};

    	// Enable EAP-PEAP authentication
    	esp_wifi_sta_wpa2_ent_enable();

    	// Set EAP identity
    	esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EXAMPLE_ESP_WIFI_USER, strlen(EXAMPLE_ESP_WIFI_USER));

    	esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EXAMPLE_ESP_WIFI_PASS, strlen(EXAMPLE_ESP_WIFI_PASS));
    }

    ...
    
}
```
This function is used to connect the ESP32 to wifi.

s_wifi_event_group = xEventGroupCreate(): This creates an event group using the FreeRTOS event group mechanism. Event groups are synchronization objects, allowing tasks to synchronize and wait for specific events to occur. Once the event group is created, it can be used to synchronize tasks in a multi-tasking environment. Tasks can wait for specific events to occur by calling functions like xEventGroupWaitBits(), and other tasks can set or clear events in the event group using functions like xEventGroupSetBits() and xEventGroupClearBits(). These are used in the event_handler() function.

The wifi_config struct contains various fields for configuring the connection to a regular wifi network with a password.

If the connection is Eduroam, a few extra fields need to be set for successful authorization to connect to it. To do this, I am redefining the wifi_config variable with the appropriate values. The password parameter now includes the username. Additionally, EAP-PEAP authentication needs to be enabled, which is the authentication that Eduroam uses. Finally, the EAP identity needs to be set, which include setting the username and password.

### Sending HTTP(S) GET requests

#### HTTP

```
static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";
```
This is the GET request to be sent to the specified WEB_PATH, WEB_SERVER and WEB_PORT.

```
static void http_get_task(void *pvParameters)
{
...
}
```
This is the task to send the specified GET request.

### HTTPS

```
static const char HOWSMYSSL_REQUEST[] = "GET " WEB_PATH " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER":"WEB_PORT"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Accept: */*\r\n"
                             "\r\n";
```
This is the GET request to be sent to the specified WEB_PATH and WEB_PORT using HTTPS.

```
static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST)
{
...
}
```
This is the function that sends the GET request using HTTPS. It is called by various functions that use different methods to send the request.

- ```https_get_request_using_crt_bundle(void)```


- ```https_get_request_using_cacert_buf(void)```


- ```https_get_request_using_global_ca_store(void)```


- ```https_get_request_to_local_server(const char* url)```


- ```https_get_request_using_already_saved_session(const char *url)```



```
static void https_request_task(void *pvparameters)
{
...
}
```
This is the task to send the specified GET request using HTTPS. It calls several of the different functions used to send the request using different methods.
