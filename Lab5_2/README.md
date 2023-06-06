# Lab 5.2

This lab is using the station example from the esp-idf GitHub repository.

https://github.com/espressif/esp-idf/tree/17451f1fb3d6485af5113d96b6836a7ce60efee8/examples/wifi/getting_started/station

## File descriptions

* ```local_server_cert.perm``` certificate needed to send HTTPS request using cacert buf, global ca store or to local server
* ```server_root_cert.perm``` certificate needed to send HTTPS request using cacert buf, global ca store or to local server
* ```station_example_main.c``` contains functionality to send requests
* ```i2c_api``` 
* ```app.py``` This is the simple web server that will be hosted on the Pi

## station_example_main.c

### POST request

```
static const char *REQUEST_POST = "POST " WEB_PATH " HTTP/1.0\r\n"
    "Host: chiara-raspberrypi:"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Content-Type: text/plain; charset=utf-8\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

static const char *POST_DATA = "Temperature is %.2fC (or %.2fF) with a %.2f%%  humidity\n";
```
This is the POST request to be sent and the data that should be sent to the specified WEB_PATH and WEB_PORT. In this case, the web server is my Pi which is named chiara-raspberrypi. This is where I am hosting a simple web server that will be able to receive POST requests. If connected to Eduroam, this will have to be adjusted to the IP address that is assigned to the Pi. This IP address can be found using the command ```ip a``` on the Pi.

```
static void http_post_task(void *pvParameters)
{
...

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
	sprintf(post_data, POST_DATA, temp, temp_f, humidity);
  
  ...
  
  int post_data_len = strlen(post_data);
  char post_request[strlen(REQUEST_POST) + post_data_len];
  sprintf(post_request, REQUEST_POST, post_data_len, post_data);

...
}
```
This is the task to send the specified POST request. It first uses the I2C api to read the temperature and humidity from the shtc3 sensor and stores the results in the post_data buffer using the POST_DATA format string and filling in the format specifiers with the measured values. It then prepares the request by filling in the format specifiers in REQUEST_POST, including the content length and post_data.

## app.py

This sets up a simple HTTP server that listens on the specified port and uses the custom request handler (MyHandler) to handle incoming POST requests and print their content to the console.

```
class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        print(post_data.decode('utf-8'))
```
This defines a custom request handler by subclassing http.server.BaseHTTPRequestHandler. In this case, we override the do_POST() method to handle POST requests. It reads the content of the POST request and prints it to the console.

```
with socketserver.TCPServer(("", PORT), MyHandler) as httpd:
    print("Server started on port", PORT)
    httpd.serve_forever()
```
This creates an instance of socketserver.TCPServer by passing an empty string as the hostname and the desired port number (PORT) along with the custom request handler class (MyHandler). By specifying an empty string as the hostname, the server will listen on all available network interfaces, enabling it to accept requests from both the local network and external network connections. The server is then started by calling the serve_forever() method. This method runs an infinite loop to handle incoming requests.

Additionally, it prints a message indicating that the server has started, including the port number on which it is listening.
