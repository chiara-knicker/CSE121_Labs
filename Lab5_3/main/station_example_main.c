/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

// Additions for HTTPS requests
/*#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "esp_timer.h"
#include "nvs.h"
#include "esp_sntp.h"
#include "esp_netif.h"*/

#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif
//#include "time_sync.h"

// I2C
#include "i2c_api.h"

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
    #define EXAMPLE_ESP_WIFI_SSID      "exampleWifi"
    #define EXAMPLE_ESP_WIFI_PASS      "examplePassword"
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#elif CONNECTION == 2
    // Home wifi
    #define EXAMPLE_ESP_WIFI_SSID      "exampleWifi"
    #define EXAMPLE_ESP_WIFI_PASS      "examplePassword"
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#elif CONNECTION == 3
    // Eduroam
    #define EXAMPLE_ESP_WIFI_SSID      "eduroam"
    #define EXAMPLE_ESP_WIFI_USER      "exampleEmail"
    #define EXAMPLE_ESP_WIFI_PASS      "examplePassword"
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

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static const char* web_server = "www.example.com";
static const char* web_port = "80";
static const char* web_url = "";
static const char* web_path = "/";

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,                                                    			&instance_got_ip));

 
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
	//wifi_config.sta.password = "exampleEmail:examplePassword";
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE;
        wifi_config.sta.pmf_cfg.capable = true;
        wifi_config.sta.pmf_cfg.required = false;

    	// Enable EAP-PEAP authentication
    	esp_wifi_sta_wpa2_ent_enable();

    	// Set EAP identity
    	esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EXAMPLE_ESP_WIFI_USER, strlen(EXAMPLE_ESP_WIFI_USER));

    	esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EXAMPLE_ESP_WIFI_PASS, strlen(EXAMPLE_ESP_WIFI_PASS));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

// ------------- For HTTP requests --------------

static const char *GET_REQUEST_FORMAT = "GET %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

char GET_REQUEST[512];

static char* http_get_request(void *pvParameters)
{
    sprintf(GET_REQUEST, GET_REQUEST_FORMAT, web_path, web_server, web_port); 
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    char* body_content = NULL;

        int err = getaddrinfo(web_server, web_port, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            return body_content;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            return body_content;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            return body_content;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, GET_REQUEST, strlen(GET_REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            return body_content;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            return body_content;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
	char* body_start = NULL;
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            /*for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
		
            }*/
            if (body_start == NULL) {
            	/* Look for the blank line signaling the start of the body */
            	body_start = strstr(recv_buf, "\r\n\r\n");
            	if (body_start != NULL) {
     	            /* Move past the blank line */
                    body_start += 4;
            	}
            }
            if (r > 0 && body_start != NULL) {
		int body_len = r - (body_start - recv_buf);
            	//printf("Body: %.*s", body_len, body_start);
		
		body_content = (char*)malloc(body_len + 1); // Allocate memory for the body content (+1 for null terminator
                strncpy(body_content, body_start, body_len); // Copy the body content into the allocated memory
                body_content[body_len] = '\0'; // Null-terminate the string

            }
        } while(r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);

	return body_content;
}

static const char *POST_REQUEST_FORMAT = "POST %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

static const char *POST_DATA = "Temperature is %.2fC (or %.2fF) with a %.2f%%  humidity\n"
    "Weather from wttr.in: %s\n";

static void http_post_request(void *pvParameters, char* weather_data)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

	// Read temperature and humidity
        uint8_t data[6] = {0,};
        uint16_t raw_humidity=0;
	uint16_t raw_temp=0;

	i2c_write_shtc3(SHTC3_CMD_MEASURE);
	i2c_read_shtc3(data, 6);

        raw_humidity = (data[3] << 8) | data[4];
	raw_temp = (data[0] << 8) | data[1];
        float humidity = calculate_humidity(raw_humidity);
	float temp = calculate_temp(raw_temp);
	float temp_f = calculate_temp_f(raw_temp);
	
	char post_data[100];
	sprintf(post_data, POST_DATA, temp, temp_f, humidity, weather_data);

	// HTTP stuff
	int err = getaddrinfo(web_server, web_port, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            return;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            return;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            return;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

	/* Prepare HTTP POST request */
	int post_data_len = strlen(post_data);
	char post_request[strlen(POST_REQUEST_FORMAT) + post_data_len + 256];
	sprintf(post_request, POST_REQUEST_FORMAT, web_path, web_server, web_port, post_data_len, post_data);

	if (write(s, post_request, strlen(post_request)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            return;
        }
        ESP_LOGI(TAG, "... socket send success");

	struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            return;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
        } while(r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
}

// ------------- For HTTPS requests --------------

static const char HOWSMYSSL_REQUEST_FORMAT[] = "GET %s HTTP/1.1\r\n"
                             "Host: %s:%s\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Accept: */*\r\n"
                             "\r\n";

char HOWSMYSSL_REQUEST[512];

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
static const char LOCAL_SRV_REQUEST[] = "GET " CONFIG_EXAMPLE_LOCAL_SERVER_URL " HTTP/1.1\r\n"
                             "Host: "WEB_SERVER"\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";
#endif

/* Root cert for howsmyssl.com, taken from server_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

extern const uint8_t local_server_cert_pem_start[] asm("_binary_local_server_cert_pem_start");
extern const uint8_t local_server_cert_pem_end[]   asm("_binary_local_server_cert_pem_end");
#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
static esp_tls_client_session_t *tls_client_session = NULL;
static bool save_client_session = false;
#endif

static char* https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST)
{
    char buf[512];
    int ret, len;

    char* body_content = NULL;

    esp_tls_t *tls = esp_tls_init();
    if (!tls) {
        ESP_LOGE(TAG, "Failed to allocate esp_tls handle!");
        goto exit;
    }

    if (esp_tls_conn_http_new_sync(WEB_SERVER_URL, &cfg, tls) == 1) {
        ESP_LOGI(TAG, "Connection established...");
    } else {
        ESP_LOGE(TAG, "Connection failed...");
        goto cleanup;
    }

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
    /* The TLS session is successfully established, now saving the session ctx for reuse */
    if (save_client_session) {
        esp_tls_free_client_session(tls_client_session);
        tls_client_session = esp_tls_get_client_session(tls);
    }
#endif

    size_t written_bytes = 0;
    do {
        ret = esp_tls_conn_write(tls,
                                 REQUEST + written_bytes,
                                 strlen(REQUEST) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            goto cleanup;
        }
    } while (written_bytes < strlen(REQUEST));

    ESP_LOGI(TAG, "Reading HTTP response...");
    do {
        len = sizeof(buf) - 1;
        memset(buf, 0x00, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        } else if (ret < 0) {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            break;
        } else if (ret == 0) {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGD(TAG, "%d bytes read", len);
	char* body_start = NULL;	
	if (body_start == NULL) {
                /* Look for the blank line signaling the start of the body */
                body_start = strstr(buf, "\r\n\r\n");
                if (body_start != NULL) {
                    /* Move past the blank line */
                    body_start += 4;
                }
            }
            if (ret > 0 && body_start != NULL) {
                int body_len = ret - (body_start - buf);
                //printf("Body: %.*s", body_len, body_start);

		body_content = (char*)malloc(body_len + 1); // Allocate memory for the body content (+1 for null terminator
		strncpy(body_content, body_start, body_len); // Copy the body content into the allocated memory
    		body_content[body_len] = '\0'; // Null-terminate the string
            }


    } while (1);

cleanup:
    esp_tls_conn_destroy(tls);
exit:
    return body_content;
}

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
static void https_get_request_using_crt_bundle(void)
{
    ESP_LOGI(TAG, "https_request using crt bundle");
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    https_get_request(cfg, web_url, HOWSMYSSL_REQUEST);
}
#endif // CONFIG_MBEDTLS_CERTIFICATE_BUNDLE

#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
static char* https_get_request_lab(void)
{
    sprintf(HOWSMYSSL_REQUEST, HOWSMYSSL_REQUEST_FORMAT, web_path, web_server, web_port);
    ESP_LOGI(TAG, "https_request using crt bundle (lab function)");
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    char* ret = https_get_request(cfg, web_url, HOWSMYSSL_REQUEST);
    return ret;
}
#endif // CONFIG_MBEDTLS_CERTIFICATE_BUNDLE


static void https_get_request_using_cacert_buf(void)
{
    ESP_LOGI(TAG, "https_request using cacert_buf");
    esp_tls_cfg_t cfg = {
        .cacert_buf = (const unsigned char *) server_root_cert_pem_start,
        .cacert_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
    };
    https_get_request(cfg, web_url, HOWSMYSSL_REQUEST);
}

static void https_get_request_using_global_ca_store(void)
{
    esp_err_t esp_ret = ESP_FAIL;
    ESP_LOGI(TAG, "https_request using global ca_store");
    esp_ret = esp_tls_set_global_ca_store(server_root_cert_pem_start, server_root_cert_pem_end - server_root_cert_pem_start);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Error in setting the global ca store: [%02X] (%s),could not complete the https_request using global_ca_store", esp_ret, esp_err_to_name(esp_ret));
        return;
    }
    esp_tls_cfg_t cfg = {
        .use_global_ca_store = true,
    };
    https_get_request(cfg, web_url, HOWSMYSSL_REQUEST);
    esp_tls_free_global_ca_store();
}

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
static void https_get_request_to_local_server(const char* url)
{
    ESP_LOGI(TAG, "https_request to local server");
    esp_tls_cfg_t cfg = {
        .cacert_buf = (const unsigned char *) local_server_cert_pem_start,
        .cacert_bytes = local_server_cert_pem_end - local_server_cert_pem_start,
        .skip_common_name = true,
    };
    save_client_session = true;
    https_get_request(cfg, url, LOCAL_SRV_REQUEST);
}

static void https_get_request_using_already_saved_session(const char *url)
{
    ESP_LOGI(TAG, "https_request using saved client session");
    esp_tls_cfg_t cfg = {
        .client_session = tls_client_session,
    };
    https_get_request(cfg, url, LOCAL_SRV_REQUEST);
    esp_tls_free_client_session(tls_client_session);
    save_client_session = false;
    tls_client_session = NULL;
}
#endif

static void lab5_3_task(void *pvparameters)
{
    char* body_content;
    while(1)
    {
    /*--------------- HTTP GET request to pi ---------------------*/
    web_server = "chiara-raspberrypi.local";
    web_port = "1234";
    web_path = "/location";

    body_content = http_get_request(NULL);
    printf("Body content GET to Pi: %s\n", body_content);

    /*----------------- HTTPS GET to wttr.in ---------------------*/
    ESP_LOGI(TAG, "Start https_request to wttr.in");

    web_server = "wttr.in";
    web_port = "443";
    char* web_url_format = "https://www.wttr.in/%s?format=%%l:+%%c+%%t";
    char web_url_temp[strlen(web_url_format) + strlen(body_content) + 1];
    sprintf(web_url_temp, web_url_format, body_content);
    web_url = web_url_temp;
    web_path = "/Santa+Cruz?format=%l:+%c+%t/";
    printf("This is the web URL: %s\n", web_url);

#ifdef CONFIG_EXAMPLE_CLIENT_SESSION_TICKETS
    char *server_url = NULL;
#ifdef CONFIG_EXAMPLE_LOCAL_SERVER_URL_FROM_STDIN
    char url_buf[SERVER_URL_MAX_SZ];
    if (strcmp(CONFIG_EXAMPLE_LOCAL_SERVER_URL, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, SERVER_URL_MAX_SZ, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        server_url = url_buf;
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: invalid url for local server");
        abort();
    }
    printf("\nServer URL obtained is %s\n", url_buf);
#else
    server_url = CONFIG_EXAMPLE_LOCAL_SERVER_URL;
#endif /* CONFIG_EXAMPLE_LOCAL_SERVER_URL_FROM_STDIN */
    https_get_request_to_local_server(server_url);
    https_get_request_using_already_saved_session(server_url);
#endif

    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());
    body_content = https_get_request_lab();
    printf("Body content GET to wttr.in: %s\n", body_content);
    //free(body_content);
    ESP_LOGI(TAG, "Finish https_request to wttr.in");

    /*--------------- HTTP POST request to pi --------------------*/
    web_server = "chiara-raspberrypi.local";
    web_port = "1234";
    web_path = "/";

    //while(1) {
    	http_post_request(NULL, body_content);
	free(body_content);
    	printf("POST request sent to Pi\n");
	vTaskDelay(pdMS_TO_TICKS(5000));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    
    i2c_master_init();


    // HTTP GET request
    //xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
  
    // HTTP POST request
    //xTaskCreate(&http_post_task, "http_post_task", 4096, NULL, 5, NULL);

    // HTTPS request
    xTaskCreate(&lab5_3_task, "lab5_3__task", 8192, NULL, 5, NULL);
}
