import socketserver
import socket

class MyUDPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        data = self.request[0].strip()
        socket = self.request[1]
        print(f"{self.client_address[0]} wrote: {data}")
        socket.sendto(data.upper(), self.client_address)

if __name__ == "__main__":
    HOST, PORT = "10.0.1.3", 1152
    with socketserver.UDPServer((HOST, PORT), MyUDPHandler) as server:
        server.serve_forever()
