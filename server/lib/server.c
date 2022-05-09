#include <stdio.h> // perror, printf
#include <stdlib.h> // exit, atoi
#include <unistd.h> // read, write, close
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <string.h> // memset
#include <sys/ioctl.h>
#include <regex.h>
#include <pthread.h>

typedef struct sockaddr_in SA_IN;
#define BUFF_SIZE 1024
#define REGEX_BROADCAST "^GET BROADCAST IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_NETWORK   "^GET NETWORK NUMBER IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_HOST "^GET HOSTS RANGE IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_RANDOM "^GET RANDOM SUBNETS NETWORK NUMBER [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9])) NUMBER [0-9]* SIZE ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
regex_t regexBroadcast;
regex_t regexNetwork;
regex_t regexHost;
regex_t regexRandom;

int compileRegex(){
    int return_value;

    return_value = regcomp(&regexBroadcast, REGEX_BROADCAST, REG_EXTENDED);
    if(return_value != 0){
        printf("Error al crear Regex Broadcast");
        return 1;
    }

    return_value = regcomp(&regexNetwork, REGEX_NETWORK, REG_EXTENDED);
    if(return_value != 0){
        printf("Error al crear Regex Network");
        return 1;
    }

    return_value = regcomp(&regexHost, REGEX_HOST, REG_EXTENDED);
    if(return_value != 0){
        printf("Error al crear Regex Host");
        return 1;
    }

    return_value = regcomp(&regexRandom, REGEX_RANDOM, REG_EXTENDED);
    if(return_value != 0){
        printf("Error al crear Regex Random");
        return 1;
    }

    return 0;
}

void selectFunction(int *tempFd){
    int clientFd = *tempFd;
    free(tempFd);
    char buffer[BUFF_SIZE];

    memset(buffer, 0, sizeof(buffer));
    int size = read(clientFd, buffer, sizeof(buffer));
    if ( size < 0 ) {
        perror("Error de lectura\n");
        exit(5);
    }
    printf("Recibido %s\n\n", buffer);
    /*
    int len;
    ioctl(clientFd, FIONREAD, &len);
    printf("Quedo por leer: %d\n",len);
    */
    int return_value;
    int visited = 0;

    char response[BUFF_SIZE];
    if(!visited && regexec(&regexBroadcast, buffer, 0, NULL, 0) == 0){
        visited = 1;
        printf("Es broadcast\n");
        strcpy(response, "BROADCAST");
    }

    if(!visited && regexec(&regexNetwork, buffer, 0, NULL, 0) == 0){
        visited = 1;
        printf("Es Network\n");
        strcpy(response, "NETWORK");
    }

    if(!visited && regexec(&regexHost, buffer, 0, NULL, 0) == 0){
        visited = 1;
        printf("Es Host\n");
        strcpy(response, "HOST");
    }

    if(!visited && regexec(&regexRandom, buffer, 0, NULL, 0) == 0){
        visited = 1;
        printf("Es Random\n");
        strcpy(response, "RANDOM");
    }
    if(!visited){
        printf("No es ninguno\n");
        strcpy(response, "NONE");
    }
    
    if (write(clientFd, response, strlen(response)) < 0) {
        perror("Error de lectura\n");
        exit(6);
    }
    close(clientFd);
}

int server(int port){
    if(compileRegex()){
        return 1;
    }
    int serverFd, clientFd;
    SA_IN server, client;
    int addr_size;

    memset(&serverFd, 0, sizeof(serverFd));

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("No se puede crear el socket\n");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    addr_size = sizeof(server);

    int activado = 1;
    setsockopt(serverFd,SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
    if (bind(serverFd, (struct sockaddr *)&server, addr_size) < 0) {
        perror("No se pudo bindear el socket\n");
        exit(2);
    }
    if (listen(serverFd, 10) < 0) {
        perror("Error de Listen\n");
        exit(3);
    }
    while (1) {
        addr_size = sizeof(client);
        printf("Esperando por clientes...\n");
        
        if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &addr_size)) < 0) {
            perror("Error en la aceptación\n");
            exit(4);
        }

        char *client_ip = inet_ntoa(client.sin_addr);
        printf("Aceptada nueva conección del cliente %s:%d\n", client_ip, ntohs(client.sin_port));
        
        int *tempFd = malloc(sizeof(int));
        *tempFd = clientFd;
        pthread_t thread;
        pthread_create(&thread, NULL, (void *) selectFunction, tempFd);
    }
    close(serverFd);
    return 0;
}

int main(int argc, char const *argv[]) {
    int port = 39801;
    
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    return server(port);
}