import http.server
import socketserver
import cgi

class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        print(post_data.decode('utf-8'))

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

PORT = 1234

with socketserver.TCPServer(("", PORT), MyHandler) as httpd:
    print("Server started on port", PORT)
    httpd.serve_forever()

