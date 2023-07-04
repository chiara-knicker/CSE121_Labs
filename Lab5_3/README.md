# Lab 5.3

This lab is based on the station example from the esp-idf GitHub repository.

https://github.com/espressif/esp-idf/tree/17451f1fb3d6485af5113d96b6836a7ce60efee8/examples/wifi/getting_started/station

## File descriptions

* ```local_server_cert.perm``` certificate needed to send HTTPS request using cacert buf, global ca store or to local server
* ```server_root_cert.perm``` certificate needed to send HTTPS request using cacert buf, global ca store or to local server
* ```station_example_main.c``` contains functionality to send requests
* ```i2c_api``` api for i2c communication and reading the icm sensor (accelerometer and gyroscope) and shct3 sensor (temperature and humidity).
* ```app.py``` simple web server that will be hosted on the Pi
* ```requests_api``` api for sending requests from ESP32

## requests_api

I put the code for connecting to wifi and sending requests into a header and source file to clean up the main file. I also made some changes to the code for sending requests to make it more reusable and flexible. The code for connecting to wifi is the same.

* ```http_get_request(void *pvParameters, const char* web_server, const char* web_port, const char* REQUEST)```
This function now takes additional arguments: web_server, web_port and REQUEST. Previously, the web server and web port had to be defined outside of the function and were then used inside the function. Adding them as parameters makes it easier to change them and send requests to different destinations because you no longer have to reassign different values to the same variables that are used inside the function, but you can have different variables for different destinations and then provide them as function arguments. The request is a function argument now for the same reason.

I also removed the while loop to fit this lab better since I now have to send different requests in a row, so I cant have an infinite loop in a function that only sends one kind of request.

Reading the HTTP response is also slightly different, since I need to extract only the body content. To do this, I iterate over the response until the blank line that indicates the start of the body is found. Once the blank line is found, the body content is stored in body_content. The body content is returned since I will have to use it later.

* ```http_post_request(void *pvParameters, const char* web_server, const char* web_port, const char* REQUEST)```
This function now takes additional arguments: web_server, web_port and REQUEST. This is for the same reason as for the http_get_request function.

I also removed the while loop in this function for the same reason as before.

* ```https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST)```
Just like for the http_get_request function, I need to extract only the body content. To do this, I iterate over the response until the blank line that indicates the start of the body is found. Once the blank line is found, the body content is stored in body_content. The body content is returned since I will have to use it later.

* ```https_get_request_using_{method}(const char* web_url, const char* REQUEST)```
I adjusted all of these functions to take additional arguments: web_url and REQUEST. This is for the same reason as for the previous functions where I added arguments. They now also return the value returned by https_get_request, which is the body content of the response, which we will need later on.

## station_example_main.c

### HTTP GET request
```
static const char *GET_REQUEST_FORMAT = "GET %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

char GET_REQUEST[512];
```
The GET_REQUEST_FORMAT includes format specifiers to make the web path, web server and port number customizable. I did this because this lab requires me to send requests to different destinations, unlike in Lab 5.1.

```
sprintf(GET_REQUEST, GET_REQUEST_FORMAT, web_path, web_server, web_port); 
```
Since the GET request is now customizable, this is how to fill in the format specifiers in GET_REQUEST_FORMAT with the appropriate web path, web server and port, and store it in GET_REQUEST.

### HTTP POST request

```
static const char *POST_REQUEST_FORMAT = "POST %s HTTP/1.0\r\n"
    "Host: %s:%s\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

static const char *POST_DATA = "Temperature is %.2fC (or %.2fF) with a %.2f%%  humidity\n"
    "Weather from wttr.in: %s\n";
```
The POST request is now also customizable for different web paths, web servers and port numbers. The post data is also customizable with the sensor readings.

```
int post_data_len = strlen(post_data);
char post_request[strlen(POST_REQUEST_FORMAT) + post_data_len + 256];
sprintf(post_request, POST_REQUEST_FORMAT, web_path, web_server, web_port, post_data_len, post_data);
```
Preparing the POST request now additionally includes specifying the web path, web server and port number.

### HTTPS GET request

```
static const char HOWSMYSSL_REQUEST_FORMAT[] = "GET %s HTTP/1.1\r\n"
                             "Host: %s:%s\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Accept: */*\r\n"
                             "\r\n";

char HOWSMYSSL_REQUEST[512];
```
The HTTPS GET request is also customizable for different web paths, web servers and port numbers.

```
sprintf(HOWSMYSSL_REQUEST, HOWSMYSSL_REQUEST_FORMAT, web_path, web_server, web_port);
```
This fills in the format specifiers in HOWSMYSSL_REQUEST_FORMAT with the appropriate web path, web server and port, and store it in HOWSMYSSL_REQUEST.

```
body_content = https_get_request_using_crt_bundle(web_url, HOWSMYSSL_REQUEST);
```
I am using the crt bundle method to send the HTTPS reques.

### lab5_3_task

This task combines all the functions for sending requests to fulfill the lab requirements. 

It first sends a HTTP GET request to the server hosted on the Pi to obtain the location stored in it. The location is obtained from the reponse and the appropriate web url is constructed to request the weather from that location from wttr.com. Then, an HTTPS GET request is sent to wttr.com with the constructed web url. The response is then stored in body_content. Lastly, the body_content is sent to the server hosted on the Pi using an HTTP POST request. 

## app.py

```
    def do_GET(self):
        if self.path == '/location':
            # Send an HTTP response with a 200 OK status code and some content
            self.send_response(200)
            self.send_header('Content-type', 'text/plain; charset=utf-8')
            self.end_headers()
            self.wfile.write(b'Santa+Cruz')

            # Log the request that was received
            print(f'Received GET request to {self.path}')

        else:
            # Send an HTTP response with a 404 Not Found status code
            self.send_error(404)
```
The server is the same as for Lab 5.2, but it can now additionally handle GET requests as well. If it receives a GET request to the /location path, it returns the specified location, in this case Santa Cruz. It cannot handle GET requests to any other paths and will return a 404 error if it receives one.

## Issues

Code:
```
char* web_url_format = "https://www.wttr.in/%s?format=%%l:+%%c+%%t";
char web_url[strlen(web_url_format) + strlen(body_content) + 1];
sprintf(web_url, web_url_format, body_content);
```
Error:
```
I (4445) wifi station: Minimum free heap size: 222580 bytes
I (4445) wifi station: https_request using crt bundle (lab function)
E (4455) esp-tls: couldn't get hostname for :: getaddrinfo() returns 202, addrinfo=0x0
E (4465) esp-tls: Failed to open new connection
E (4475) wifi station: Connection failed...
Guru Meditation Error: Core  0 panic'ed (Load access fault). Exception was unhandled.

Stack dump detected
Core  0 register dump:
MEPC    : 0x40058eb6  RA      : 0x42099aee  SP      : 0x3fcaa220  GP      : 0x3fc92200
0x42099aee: _vfprintf_r at /builds/idf/crosstool-NG/.build/HOST-aarch64-linux-gnu/riscv32-esp-elf/src/newlib/newlib/libc/stdio/vfprintf.c:1524

TP      : 0x3fc76ec4  T0      : 0x42046746  T1      : 0x3fcaa2dc  T2      : 0xffffffff
0x42046746: mbedtls_platform_zeroize at /home/chiar/CSE121_Labs/esp-idf/components/mbedtls/mbedtls/library/platform_util.c:73

S0/FP   : 0x00000000  S1      : 0x00000000  A0      : 0x00000000  A1      : 0xffffffff
A2      : 0x0000001f  A3      : 0x7f7f7f7f  A4      : 0x00000000  A5      : 0x00000004
A6      : 0x00000001  A7      : 0x0000000a  S2      : 0x0000005a  S3      : 0x00000000
S4      : 0x00000001  S5      : 0x3fc998e8  S6      : 0xffffffff  S7      : 0x3fcaa2e4
S8      : 0x00000000  S9      : 0x00000073  S10     : 0x00000000  S11     : 0x00000000
T3      : 0x00000000  T4      : 0x00000000  T5      : 0x0ccccccc  T6      : 0x00000019
MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000005  MTVAL   : 0x00000000
0x40380001: _vector_table at ??:?

MHARTID : 0x00000000


Backtrace:


0x40058eb6 in ?? ()
#0  0x40058eb6 in ?? ()
#1  0x42099aee in _vfprintf_r (data=<optimized out>, fp=0x3fc998e8, fmt0=<optimized out>, ap=<optimized out>) at /builds/idf/crosstool-NG/.build/HOST-aarch64-linux-gnu/riscv32-esp-elf/src/newlib/newlib/libc/stdio/vfprintf.c:1524
#2  0x00000000 in ?? ()
Backtrace stopped: frame did not save the PC
ELF file SHA256: 534576e6068b915a

Rebooting...
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x3 (RTC_SW_SYS_RST),boot:0xc (SPI_FAST_FLASH_BOOT)
Saved PC:0x40048b82
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5820,len:0x1718
load:0x403cc710,len:0x974
load:0x403ce710,len:0x2f68
entry 0x403cc710
```
Fix:
```
char* web_url_format = "https://www.wttr.in/%s?format=%%l:+%%c+%%t";
char web_url_temp[strlen(web_url_format) + strlen(body_content) + 1];
sprintf(web_url_temp, web_url_format, body_content);
web_url = web_url_temp;
```
Explanation:

In the first code snippet, web_url is an array of characters that is being dynamically allocated on the stack using the sprintf function. Its size is calculated based on the lengths of web_url_format and body_content. This array stores a modified version of the URL with the format specifier %s replaced by the value of body_content. web_url is an array of characters that can be modified.

web_url should be of type `static const char*`, a pointer to a string literal. String literals are typically stored in read-only memory, and web_url is pointing to that memory location. It is a read-only string literal.

The second code snippet turns web_url_temp into a string literal by assigning it to web_url.
