# echo-client.py
import socket

HOST = "186.177.110.87"  # The server's hostname or IP address
PORT = 9801  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    send_data = input("Ingrese mensaje\n");
    s.sendall(bytes(send_data, 'UTF-8'))
    data = s.recv(1024)

print(f"Received {data!r}")