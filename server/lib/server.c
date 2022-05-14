#include <stdio.h>     // perror, printf
#include <stdlib.h>    // exit, atoi
#include <unistd.h>    // read, write, close
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <string.h>    // memset
#include <sys/ioctl.h>
#include <regex.h>
#include <pthread.h>
#include <inttypes.h>

typedef struct sockaddr_in SA_IN;
#define BUFF_SIZE 1024
#define REGEX_BROADCAST "^GET BROADCAST IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_NETWORK "^GET NETWORK NUMBER IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_HOST "^GET HOSTS RANGE IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_RANDOM "^GET RANDOM SUBNETS NETWORK NUMBER [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9])) NUMBER [0-9]* SIZE ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_MASK "^/[0-9][0-9]?\r?\n?$"
#define REGEX_MASK2 "^[0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]\r?\n?$"
regex_t regexBroadcast;
regex_t regexNetwork;
regex_t regexHost;
regex_t regexRandom;
regex_t regexMask;
regex_t regexMask2;

struct IP
{
    uint8_t firstByte;
    uint8_t secondByte;
    uint8_t thirdByte;
    uint8_t fourByte;
} typedef IP;
/**
 * @brief Crea un objeto IP
 * 
 * @param mask Si es 0 se inicializa cada byte en 0, de lo contrario se inicializa cada byte con 255
 * @return IP* Nuevo IP recien creado.
 */
IP *createIp(int mask) {
    IP *newIp = malloc(sizeof(IP));
    uint8_t defaultValue = 0;
    if (mask)
    {
        defaultValue = 255;
    }

    newIp->firstByte = defaultValue;
    newIp->secondByte = defaultValue;
    newIp->thirdByte = defaultValue;
    newIp->fourByte = defaultValue;
    return newIp;
}
/**
 * @brief Compila los regex necesarios para el servidor
 * 
 * @return int 0 si se ejecuto correctamente, de lo contrario es un codigo de error.
 */
int compileRegex() {
    int return_value;

    return_value = regcomp(&regexBroadcast, REGEX_BROADCAST, REG_EXTENDED);
    if (return_value != 0)
    {
        printf("Error al crear Regex Broadcast\n");
        return 1;
    }

    return_value = regcomp(&regexNetwork, REGEX_NETWORK, REG_EXTENDED);
    if (return_value != 0)
    {
        printf("Error al crear Regex Network\n");
        return 1;
    }

    return_value = regcomp(&regexHost, REGEX_HOST, REG_EXTENDED);
    if (return_value != 0)
    {
        printf("Error al crear Regex Host\n");
        return 1;
    }

    return_value = regcomp(&regexRandom, REGEX_RANDOM, REG_EXTENDED);
    if (return_value != 0)
    {
        printf("Error al crear Regex Random\n");
        return 1;
    }

    return_value = regcomp(&regexMask, REGEX_MASK, REG_EXTENDED);
    if (return_value != 0)
    {
        printf("Error al crear Regex Mask\n");
        return 1;
    }

    return_value = regcomp(&regexMask2, REGEX_MASK2, REG_EXTENDED);
    if (return_value != 0)
    {
        printf("Error al crear Regex Mask2\n");
        return 1;
    }

    return 0;
}

/**
 * @brief Transforma un String que contiene un IP a un struct IP
 * 
 * @param str String que contiene el ip en formato XXX.XXX.XXX.XXX
 * @param ip Estructura de datos de IP donde se almacenara los datos obtenidos
 * @return int 0 si los bytes estan bien, un valor mayor a 0 si excede los limites de un byte 
 * Codigos de error:
 * 1: Byte 1 con mal formato
 * 2: Byte 2 con mal formato
 * 3: Byte 3 con mal formato
 * 4: Byte 4 con mal formato
 */
int str2ip(char *str, IP *ip)
{
    char *byte;
    byte = strtok(str, ".");
    int temp = atoi(byte);
    if (temp < 0 || temp > 255) {
        return 1;
    }
    ip->firstByte = temp;

    byte = strtok(NULL, ".");
    temp = atoi(byte);
    if (temp < 0 || temp > 255) {
        return 2;
    }
    ip->secondByte = temp;

    byte = strtok(NULL, ".");
    temp = atoi(byte);
    if (temp < 0 || temp > 255) {
        return 3;
    }
    ip->thirdByte = temp;

    byte = strtok(NULL, ".");
    temp = atoi(byte);
    if (temp < 0 || temp > 255) {
        return 4;
    }
    ip->fourByte = temp;
    return 0;
}
/**
 * @brief Transforma un String que contiene una mascara de red a una estructura IP que contendra la mascara de red
 * 
 * @param str String que contiene la mascara de red con formato: /XX o XXX.XXX.XXX.XXX
 * @param ip Estructura de datos donde se almacenara la mascara de red
 * @return int int 0 si los bytes estan bien, un valor mayor a 0 si excede los limites de un byte 
 * Codigos de error:
 * 5: Mal formato en /XX. Es menor a 8 o mayor a 32
 * 6: Byte 1 con mal formato
 * 7: Byte 2 con mal formato
 * 8: Byte 3 con mal formato
 * 9: Byte 4 con mal formato
 * 10: Mal formato. No es ni /XX ni XXX.XXX.XXX.XXX
 * 11: Los bits de la mascara son menores 8
 */
int str2mask(char *str, IP *ip) {
    if (regexec(&regexMask, str, 0, NULL, 0) == 0) {
        int str_len = strlen(str);
        if (str[str_len-1] == '\n') {
            str[str_len-1] = '\0';
        }
        str_len = strlen(str);
        if (str[str_len-1] == '\r') {
            str[str_len-1] = '\0';
        }
        int mask = atoi(str+1);
        if(mask < 8 || mask > 32){
            return 5;
        }
        int diff = 32-mask;
        
        if (diff < 9) {
            ip->fourByte = ((255 >> diff) << diff);
            return 0;
        }
        if (diff < 17) {
            ip->fourByte = 0;
            ip->thirdByte = ((255 >> diff) << diff);
            return 0;
        }
        if (diff < 25) {
            ip->fourByte = 0;
            ip->thirdByte = 0;
            ip->secondByte = ((255 >> diff) << diff);
            return 0;
        }
        return 11;
    }
    if (regexec(&regexMask2, str, 0, NULL, 0) == 0) {
        int str_len = strlen(str);
        if (str[str_len-1] == '\n') {
            str[str_len-1] = '\0';
        }
        str_len = strlen(str);
        if (str[str_len-1] == '\r') {
            str[str_len-1] = '\0';
        }
        int return_value = str2ip(str, ip);
        if(return_value){
            return 5 + return_value;
        }
        return 0;
    }
    return 10;
}
/**
 * @brief Calcula el complemento(negación) a un IP
 * 
 * @param ip Estructura que representa un IP
 */
void bitCompliment(IP *ip) {
    ip->firstByte = ~ip->firstByte;
    ip->secondByte = ~ip->secondByte;
    ip->thirdByte = ~ip->thirdByte;
    ip->fourByte = ~ip->fourByte;
}
/**
 * @brief Transforma un String en un ip y mascara de red
 * 
 * @param str String que contiene el ip y mascara de red. Formato: 'XXX.XXX.XXX.XXX MASK /XX' o 'XXX.XXX.XXX.XXX MASK XXX.XXX.XXX.XXX'
 * @param ip Estructura que contendra el IP obtenido
 * @param netMask Estructura que contendra la mascara de red obtenida
 * @param iWord Cantidad de palabras antes del IP
 * 
 * @return int 0 Si se ejecuto correctamente, de lo contrario es un codigo de error
 */
int str2IpMask(char *str, IP *ip, IP *netMask, int iWord) {
    char *token;
    char *save_ptr1;
    // Hago tres strtok para quitar GET BROADCAST IP
    token = strtok_r(str, " ",&save_ptr1);
    for (int i = 0; i < iWord; i++) {
        token = strtok_r(NULL, " ",&save_ptr1);
    }

    int return_value = str2ip(token, ip);
    if (return_value){
        return return_value;
    }
    //printf("IP: %d.%d.%d.%d\n", ip->firstByte, ip->secondByte, ip->thirdByte, ip->fourByte);

    token = strtok_r(NULL, " ",&save_ptr1);
    token = strtok_r(NULL, " ",&save_ptr1);
    return_value = str2mask(token,netMask);
    if (return_value){
        return return_value;
    }
    return 0;
    //printf("NETMASK: %d.%d.%d.%d\n", netMask->firstByte, netMask->secondByte, netMask->thirdByte, netMask->fourByte);
}
/**
 * @brief Aplica la operación bitwise OR entre dos IP
 * 
 * @param ip1 Primer IP a aplicar OR
 * @param ip2 Segundo IP a aplicar OR
 * 
 * @return IP* Nuevo IP resultado del OR
 */
IP *ipOr(IP *ip1, IP *ip2) {
    IP *newIp = createIp(0);
    newIp->firstByte = ip1->firstByte | ip2->firstByte;
    newIp->secondByte = ip1->secondByte | ip2->secondByte;
    newIp->thirdByte = ip1->thirdByte | ip2->thirdByte;
    newIp->fourByte = ip1->fourByte | ip2->fourByte;
    return newIp;
}
/**
 * @brief Aplica la operación bitwise AND entre dos IP
 * 
 * @param ip1 Primer IP a aplicar AND
 * @param ip2 Segundo IP a aplicar AND
 * @return IP* Nuevo IP resultado del AND
 */
IP *ipAnd(IP *ip1, IP *ip2) {
    IP *newIp = createIp(0);
    newIp->firstByte = ip1->firstByte & ip2->firstByte;
    newIp->secondByte = ip1->secondByte & ip2->secondByte;
    newIp->thirdByte = ip1->thirdByte & ip2->thirdByte;
    newIp->fourByte = ip1->fourByte & ip2->fourByte;
    return newIp;
}

void sendError(int clientFd, int return_value){
    char response[BUFF_SIZE];
    memset(response, 0, sizeof(response));
    switch (return_value) {
        case 1:
            strcpy(response, "Byte 1 de IP con mal formato\n");
            break;
        case 2:
            strcpy(response, "Byte 2 de IP con mal formato\n");
            break;
        case 3:
            strcpy(response, "Byte 3 de IP con mal formato\n");
            break;
        case 4:
            strcpy(response, "Byte 4 de IP con mal formato\n");
            break;
        case 5:
            strcpy(response, "Mal formato en /XX. Es menor a 8 o mayor a 32\n");
            break;
        case 6:
            strcpy(response, "Byte 1 de Mascara de Red con mal formato\n");
            break;
        case 7:
            strcpy(response, "Byte 2 de Mascara de Red con mal formato\n");
            break;
        case 8:
            strcpy(response, "Byte 3 de Mascara de Red con mal formato\n");
            break;
        case 9:
            strcpy(response, "Byte 4 de Mascara de Red con mal formato\n");
            break;
        case 10:
            strcpy(response, "Mal formato. No es ni /XX ni XXX.XXX.XXX.XXX\n");
            break;
        case 11:
            strcpy(response, "Los bits de la mascara son menores 8\n");
            break;
        case 12:
            strcpy(response, "No corresponde a ninguna de las 4 funcionalidades. Revise el formato:\n\nGET BROADCAST IP {dirección IP} MASK {/bits o X.X.X.X}\nGET NETWORK NUMBER IP {dirección IP} MASK {/bits o X.X.X.X}\nGET HOSTS RANGE IP {dirección IP} MASK {/bits o X.X.X.X}\nGET RANDOM SUBNETS NETWORK NUMBER {Y.Y.Y.Y} MASK {/bits o X.X.X.X} NUMBER {número de redes} SIZE {/bits o X.X.X.X}\n");
            break;
        default:
            strcpy(response, "Error inesperado\n");
            break;
    }
    if (write(clientFd, response, sizeof(response)) < 0) {
        perror("Error de lectura\n");
        close(clientFd);
        exit(6);
    }
}
/**
 * @brief Selecciona la funcionalidad que le da el cliente y la ejecuta.
 * 
 * @param tempFd File Descriptor del socket de comunicación del cliente.
 */
void selectFunction(int *tempFd) {
    int clientFd = *tempFd;
    free(tempFd);
    char buffer[BUFF_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int size = recv(clientFd, buffer, BUFF_SIZE, 0);
        if (!size) break;
        if (size < 0) {
            perror("Error de lectura\n");
            close(clientFd);
            exit(5);
        }
        printf("Recibido %s\n\n", buffer);
        int return_value;
        int visited = 0;

        char response[BUFF_SIZE];
        memset(response, 0, sizeof(response));

        IP *ip = createIp(0);
        IP *netMask = createIp(1);

        if (!visited && regexec(&regexBroadcast, buffer, 0, NULL, 0) == 0) {
            visited = 1;
            int return_value = str2IpMask(buffer,ip,netMask,3);
            if(return_value){
                sendError(clientFd, return_value);
                continue;
            }
            bitCompliment(netMask);
            IP *broadcastIp = ipOr(ip,netMask);
            sprintf(response, "%d.%d.%d.%d\n", broadcastIp->firstByte, broadcastIp->secondByte, broadcastIp->thirdByte, broadcastIp->fourByte);
            free(broadcastIp);
        }

        if (!visited && regexec(&regexNetwork, buffer, 0, NULL, 0) == 0) {
            visited = 1;
            int return_value = str2IpMask(buffer,ip,netMask,4);
            if(return_value){
                sendError(clientFd, return_value);
                continue;
            }
            IP *gateway = ipAnd(ip,netMask);
            sprintf(response, "%d.%d.%d.%d\n", gateway->firstByte, gateway->secondByte, gateway->thirdByte, gateway->fourByte);
            free(gateway);
        }

        if (!visited && regexec(&regexHost, buffer, 0, NULL, 0) == 0) {
            visited = 1;
            printf("Es Host\n");
            int return_value = str2IpMask(buffer,ip,netMask,4);
            if(return_value){
                sendError(clientFd, return_value);
                continue;
            }
            IP *gateway = ipAnd(ip,netMask);
            bitCompliment(netMask);
            char *range1 = malloc(sizeof(char)*20);
            char *range2 = malloc(sizeof(char)*20);
            char *range3 = malloc(sizeof(char)*20);
            char *range4 = malloc(sizeof(char)*20);
            sprintf(range1, "%d", gateway->firstByte);
            sprintf(range2, "%d", gateway->secondByte);
            sprintf(range3, "%d", gateway->thirdByte);
            sprintf(range4, "%d", gateway->fourByte);
            if (netMask->fourByte != 0){
                sprintf(range4, "{1-%d}", netMask->fourByte-1);
            }
            if (netMask->thirdByte != 0){
                sprintf(range3, "{0-%d}", netMask->thirdByte);
            }
            if (netMask->secondByte != 0){
                sprintf(range2, "{0-%d}", netMask->secondByte);
            }

            sprintf(response, "%s.%s.%s.%s\n", range1,range2,range3,range4);
        }

        if (!visited && regexec(&regexRandom, buffer, 0, NULL, 0) == 0) {
            visited = 1;
            printf("Es Random\n");
            strcpy(response, "RANDOM");
        }
        if (!visited) {
            printf("No es ninguno\n");
            sendError(clientFd, 12);
            continue;
        }

        free(ip);
        free(netMask);

        if (write(clientFd, response, sizeof(response)) < 0) {
            perror("Error de lectura\n");
            close(clientFd);
            exit(6);
        }
    }
    close(clientFd);
}
/**
 * @brief Funcion principal que actua como servidor. Crea hilos para atender a los clientes.
 * 
 * @param port Puerto por el que el servidor va a recibir comunicaciones.
 * @return int Codigo de retorno, si es 0 se ejecuto correctamente y si es mayor a 0 es un codigo de error.
 */
int server(int port) {
    if (compileRegex()) {
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
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
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

        if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &addr_size)) < 0)
        {
            perror("Error en la aceptación\n");
            exit(4);
        }

        char *client_ip = inet_ntoa(client.sin_addr);
        printf("Aceptada nueva conección del cliente %s:%d\n", client_ip, ntohs(client.sin_port));

        int *tempFd = malloc(sizeof(int));
        *tempFd = clientFd;
        pthread_t thread;
        pthread_create(&thread, NULL, (void *)selectFunction, tempFd);
    }
    close(serverFd);
    return 0;
}
/**
 * @brief Funcion principal que inicia el servidor.
 */
int main(int argc, char const *argv[]) {
    int port = 39801;

    if (argc == 2) {
        port = atoi(argv[1]);
    }
    return server(port);
}