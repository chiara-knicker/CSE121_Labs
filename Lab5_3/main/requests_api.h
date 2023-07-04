#ifndef REQUESTS_API_H
#define REQUESTS_API_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// Additions for HTTP requests
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

// I2C
#include "i2c_api.h"

// Passwords
#include "../../config.h"

// --------------------------- DEFINITIONS ----------------------------

/* Constants that aren't configurable in menuconfig */
#define SERVER_URL_MAX_SZ 256

/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD (86400000000ULL)

#define CONNECTION 2

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

#if CONNECTION == 0
    // Configured wifi
    #define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
    #define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#elif CONNECTION == 1
    // Phone hotspot
    #define EXAMPLE_ESP_WIFI_SSID      NAME_WIFI_HOTSPOT
    #define EXAMPLE_ESP_WIFI_PASS      PASSWORD_WIFI_HOTSPOT
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#elif CONNECTION == 2
    // Home wifi
    #define EXAMPLE_ESP_WIFI_SSID      NAME_WIFI_HOME
    #define EXAMPLE_ESP_WIFI_PASS      PASSWORD_WIFI_HOME
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#elif CONNECTION == 3
    // Eduroam
    #define EXAMPLE_ESP_WIFI_SSID      "eduroam"
    #define EXAMPLE_ESP_WIFI_USER      EMAIL_WIFI_EDUROAM
    #define EXAMPLE_ESP_WIFI_PASS      PASSWORD_WIFI_EDUROAM
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY 
#endif

#ifndef EXAMPLE_ESP_WIFI_USER
#define EXAMPLE_ESP_WIFI_USER 	       "default"
#endif

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// ------------------------- WIFI --------------------------

void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

void wifi_init_sta(void);

// ------------- For HTTP requests --------------

char* http_get_request(void *pvParameters, const char* web_server, const char* web_port, const char* REQUEST);

void http_post_request(void *pvParameters, const char* web_server, const char* web_port, const char* REQUEST);

// ------------- For HTTPS requests --------------

char* https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST);

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
char* https_get_request_using_crt_bundle(const char* web_url, const char* REQUEST);
#endif // CONFIG_MBEDTLS_CERTIFICATE_BUNDLE

char* https_get_request_using_cacert_buf(const char* web_url, const char* REQUEST);

char* https_get_request_using_global_ca_store(const char* web_url, const char* REQUEST);

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
void https_get_request_to_local_server(const char* url);

void https_get_request_using_already_saved_session(const char *url);
#endif

#endif
