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
This function handles various events:
- connecting to wifi
- retrying to connect to wifi if maximum number of retries has not been reached, otherwise connection failed
- received IP address

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

s_wifi_event_group = xEventGroupCreate(): [TODO: explain what this does]

wifi_config stores various configuration parameters for connection to a regular wifi network with a password. 

If the connection is Eduroam, a few extra parameters need to be set for successful authorization to connect to it. To do this, I am redefining the wifi_config variable with the appropriate parameters. The password parameter now includes the username. Additionally, EAP-PEAP authentication needs to be enabled, which is the authentication that Eduroam uses. Finally, the EAP identity needs to be set, which include setting the username and password.

### Sending HTTP request

#### GET request

#### POST request

### Sending HTTPS request

