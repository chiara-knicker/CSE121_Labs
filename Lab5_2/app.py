import http.server
import socketserver
import cgi

class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        print(post_data.decode('utf-8'))

PORT = 1234

with socketserver.TCPServer(("", PORT), MyHandler) as httpd:
    print("Server started on port", PORT)
    httpd.serve_forever()

