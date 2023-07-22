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
//#include "time_sync.h"

// I2C
#include "i2c_api.h"

// Requests
#include "requests_api.h"

static const char *TAG = "wifi station";

static const char* web_server = "www.example.com";
static const char* web_port = "80";
static const char* web_url = "";
static const char* web_path = "/";

// ------------- For HTTP requests --------------

static const char *GET_REQUEST_FORMAT = "GET %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

char GET_REQUEST[512];

static const char *POST_REQUEST_FORMAT = "POST %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

static const char *POST_DATA = "Temperature is %.2fC (or %.2fF) with a %.2f%%  humidity\n"
    "Weather from wttr.in: %s\n";

// ------------- For HTTPS requests --------------

static const char HOWSMYSSL_REQUEST_FORMAT[] = "GET %s HTTP/1.1\r\n"
                             "Host: %s:%s\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Accept: */*\r\n"
                             "\r\n";

char HOWSMYSSL_REQUEST[512];

static void lab5_3_task(void *pvparameters)
{
    char* body_content;
    for(int i = 0; i<3; i++)
    {
    /*--------------- HTTP GET request to pi ---------------------*/
    web_server = "Chiara-Notebook";
    web_port = "1234";
    web_path = "/location";

    sprintf(GET_REQUEST, GET_REQUEST_FORMAT, web_path, web_server, web_port);

    body_content = http_get_request(NULL, web_server, web_port, GET_REQUEST);
    printf("Server Response (Location): %s\n", body_content);

    /*----------------- HTTPS GET to wttr.in ---------------------*/
    ESP_LOGI(TAG, "Start https_request to wttr.in");

    web_server = "wttr.in";
    web_port = "443";
    char* web_url_format = "https://www.wttr.in/%s?format=%%l:+%%c+%%t";
    char web_url_temp[strlen(web_url_format) + strlen(body_content) + 1];
    sprintf(web_url_temp, web_url_format, body_content);
    web_url = web_url_temp;
    char* web_path_format = "/%s?format=%%l:+%%c+%%t/";
    char web_path_temp[strlen(web_path_format) + strlen(body_content) + 1];
    sprintf(web_path_temp, web_path_format, body_content);
    web_path = web_path_temp;
    printf("This is the web URL: %s\n", web_url);
    printf("This is the web path: %s\n", web_path);

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

    sprintf(HOWSMYSSL_REQUEST, HOWSMYSSL_REQUEST_FORMAT, web_path, web_server, web_port);
    body_content = https_get_request_using_crt_bundle(web_url, HOWSMYSSL_REQUEST);
    printf("Response from wttr.in: %s\n", body_content);
    //free(body_content);
    ESP_LOGI(TAG, "Finish https_request to wttr.in");

    /*--------------- HTTP POST request to pi --------------------*/
    web_server = "Chiara-Notebook";
    web_port = "1234";
    web_path = "/";

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
    sprintf(post_data, POST_DATA, temp, temp_f, humidity, body_content);

    /* Prepare HTTP POST request */
    int post_data_len = strlen(post_data);
    char post_request[strlen(POST_REQUEST_FORMAT) + post_data_len + 256];
    sprintf(post_request, POST_REQUEST_FORMAT, web_path, web_server, web_port, post_data_len, post_data);

    //while(1) {
    	http_post_request(NULL, web_server, web_port, post_request);
	free(body_content);
    	printf("POST request sent to Server\n");
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

    // HTTPS request
    xTaskCreate(&lab5_3_task, "lab5_3__task", 8192, NULL, 5, NULL);
}
