# Cliente de pruebas
import socket

HOST = "127.0.0.1"
PORT = 9801

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    send_data = input("Ingrese mensaje\n");
    s.sendall(bytes(send_data, 'UTF-8'))
    data = s.recv(1024)

print(f'Recibido {data!r}')