#include <stdio.h>     // perror, printf
#include <stdlib.h>    // exit, atoi
#include <unistd.h>    // read, write, close
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <string.h>    // memset
#include <sys/ioctl.h>
#include <regex.h>
#include <pthread.h>

typedef struct sockaddr_in SA_IN;
#define BUFF_SIZE 1024
#define REGEX_BROADCAST "^GET BROADCAST IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_NETWORK "^GET NETWORK NUMBER IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_HOST "^GET HOSTS RANGE IP [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_RANDOM "^GET RANDOM SUBNETS NETWORK NUMBER [0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9].[0-9]?[0-9]?[0-9] MASK ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9])) NUMBER [0-9]* SIZE ((/[0-9][0-9]?)|([0-9]{3}.([0-9][0-9])?[0-9].([0-9][0-9])?[0-9].([0-9][0-9])?[0-9]))\r?\n$"
#define REGEX_MASK "^/[0-9][0-9]?$"
regex_t regexBroadcast;
regex_t regexNetwork;
regex_t regexHost;
regex_t regexRandom;
regex_t regexMask;

struct IP
{
    int firstByte;
    int secondByte;
    int thirdByte;
    int fourByte;
} typedef IP;

IP *createIp(int mask)
{
    IP *newIp = malloc(sizeof(IP));
    int defaultValue = 0;
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

int compileRegex()
{
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

    return 0;
}

unsigned int ip_to_int(const char *ip)
{
    /* The return value. */
    unsigned resultIp = 0;
    /* The count of the number of bytes processed. */
    int bytesIndex;
    /* A pointer to the next digit to process. */
    const char *nextDigit;

    nextDigit = ip;
    for (bytesIndex = 0; bytesIndex < 4; bytesIndex++)
    {
        /* The digit being processed. */
        char digit;
        /* The value of this byte. */
        int digitValue = 0;
        while (1)
        {
            digit = *nextDigit;
            nextDigit++;
            if (digit >= '0' && digit <= '9')
            {
                digitValue *= 10;
                digitValue += digit - '0';
            }
            /* We insist on stopping at "." if we are still parsing
               the first, second, or third numbers. If we have reached
               the end of the numbers, we will allow any character. */
            else if ((bytesIndex < 3 && digit == '.') || bytesIndex == 3)
            {
                break;
            }
            else
            {
                return -1;
            }
        }
        if (digitValue >= 256)
        {
            return -1;
        }
        resultIp *= 256;
        resultIp += digitValue;
    }
    return resultIp;
}

void str2ip(char *str, IP *ip)
{
    char *byte;
    byte = strtok(str, ".");
    ip->firstByte = atoi(byte);

    byte = strtok(NULL, ".");
    ip->secondByte = atoi(byte);

    byte = strtok(NULL, ".");
    ip->thirdByte = atoi(byte);

    byte = strtok(NULL, ".");
    ip->fourByte = atoi(byte);
}

void selectFunction(int *tempFd)
{
    int clientFd = *tempFd;
    free(tempFd);
    char buffer[BUFF_SIZE];

    memset(buffer, 0, sizeof(buffer));
    int size = read(clientFd, buffer, sizeof(buffer));
    if (size < 0)
    {
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

    IP *ip = createIp(0);
    IP *netMask = createIp(1);

    if (!visited && regexec(&regexBroadcast, buffer, 0, NULL, 0) == 0)
    {
        visited = 1;
        printf("Es broadcast\n");
        char *token;
        // Hago tres strtok para quitar GET BROADCAST IP
        token = strtok(buffer, " ");
        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        token = strtok(NULL, " ");

        str2ip(token, ip);
        printf("IP: %d.%d.%d.%d\n", ip->firstByte, ip->secondByte, ip->thirdByte, ip->fourByte);

        strcpy(response, "BROADCAST");
    }

    if (!visited && regexec(&regexNetwork, buffer, 0, NULL, 0) == 0)
    {
        visited = 1;
        printf("Es Network\n");
        strcpy(response, "NETWORK");
    }

    if (!visited && regexec(&regexHost, buffer, 0, NULL, 0) == 0)
    {
        visited = 1;
        printf("Es Host\n");
        strcpy(response, "HOST");
    }

    if (!visited && regexec(&regexRandom, buffer, 0, NULL, 0) == 0)
    {
        visited = 1;
        printf("Es Random\n");
        strcpy(response, "RANDOM");
    }
    if (!visited)
    {
        printf("No es ninguno\n");
        strcpy(response, "NONE");
    }

    if (write(clientFd, response, strlen(response)) < 0)
    {
        perror("Error de lectura\n");
        exit(6);
    }
    close(clientFd);
}

int server(int port)
{
    if (compileRegex())
    {
        return 1;
    }
    int serverFd, clientFd;
    SA_IN server, client;
    int addr_size;

    memset(&serverFd, 0, sizeof(serverFd));

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        perror("No se puede crear el socket\n");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    addr_size = sizeof(server);

    int activado = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
    if (bind(serverFd, (struct sockaddr *)&server, addr_size) < 0)
    {
        perror("No se pudo bindear el socket\n");
        exit(2);
    }
    if (listen(serverFd, 10) < 0)
    {
        perror("Error de Listen\n");
        exit(3);
    }
    while (1)
    {
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

int main(int argc, char const *argv[])
{
    int port = 39801;

    if (argc == 2)
    {
        port = atoi(argv[1]);
    }
    return server(port);
}