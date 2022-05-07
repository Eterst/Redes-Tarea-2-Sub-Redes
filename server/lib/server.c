#include <stdio.h> // perror, printf
#include <stdlib.h> // exit, atoi
#include <unistd.h> // read, write, close
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <string.h> // memset

typedef struct sockaddr_in SA_IN;

void echo(int clientFd){
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));
    int size = read(clientFd, buffer, sizeof(buffer));
    if ( size < 0 ) {
        perror("Error de lectura");
        exit(5);
    }

    printf("Recibido %s del cliente\n", buffer);
    if (write(clientFd, buffer, size) < 0) {
        perror("Error de lectura");
        exit(6);
    }
    close(clientFd);
}

int main(int argc, char const *argv[]) {
    int serverFd, clientFd;
    SA_IN server, client;
    int addr_size;
    int port = 39801;
    
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    memset(&serverFd, 0, sizeof(serverFd));

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("No se puede crear el socket");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    addr_size = sizeof(server);

    int activado = 1;
    setsockopt(serverFd,SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
    if (bind(serverFd, (struct sockaddr *)&server, addr_size) < 0) {
        perror("No se pudo bindear el socket");
        exit(2);
    }
    if (listen(serverFd, 10) < 0) {
        perror("Error de Listen");
        exit(3);
    }
    while (1) {
        addr_size = sizeof(client);
        printf("Esperando por clientes...\n");
        
        if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &addr_size)) < 0) {
            perror("Error en la aceptación");
            exit(4);
        }

        char *client_ip = inet_ntoa(client.sin_addr);
        printf("Aceptada nueva conección del cliente %s:%d\n", client_ip, ntohs(client.sin_port));
        
        echo(clientFd);
    }
    close(serverFd);
    return 0;
}