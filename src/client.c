#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVEURNAME "192.168.50.177"  // <- mettre l'IP réelle du serveur
#define PORT 30000
#define BUFFERSIZE 512

int main(void) {
    char buffer[BUFFERSIZE];
    char pseudo[50];
    int to_server_socket;
    long hostAddr;
    struct sockaddr_in serverSockAddr;
    struct hostent *serverHostEnt;

    memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    hostAddr = inet_addr(SERVEURNAME);
    if (hostAddr != (long)-1) {
        memcpy(&serverSockAddr.sin_addr, &hostAddr, sizeof(hostAddr));
    } else {
        serverHostEnt = gethostbyname(SERVEURNAME);
        if (serverHostEnt == NULL) {
            perror("gethostbyname");
            exit(EXIT_FAILURE);
        }
        memcpy(&serverSockAddr.sin_addr, serverHostEnt->h_addr, serverHostEnt->h_length);
    }

    serverSockAddr.sin_port = htons(PORT);
    serverSockAddr.sin_family = AF_INET;

    if ((to_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (connect(to_server_socket, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Entrez votre pseudo : ");
    scanf("%49s", pseudo);

    snprintf(buffer, sizeof(buffer), "HELLO %s\n", pseudo);
    write(to_server_socket, buffer, strlen(buffer));

    memset(buffer, 0, sizeof(buffer));
    read(to_server_socket, buffer, sizeof(buffer) - 1);
    printf("%s", buffer);

    while (1) {
        int x, y;
        printf("Entrez les coordonnées du tir (x y) : ");
        scanf("%d %d", &x, &y);
        while (getchar() != '\n'); // vider le buffer stdin

        snprintf(buffer, sizeof(buffer), "SHOOT %d %d\n", x, y);
        write(to_server_socket, buffer, strlen(buffer));

        memset(buffer, 0, sizeof(buffer));
        read(to_server_socket, buffer, sizeof(buffer) - 1);
        printf("Réponse serveur: %s", buffer);

        if (strncmp(buffer, "WIN", 3) == 0) {
            printf("Vous avez gagné !\n");
            break;
        }
    }

    snprintf(buffer, sizeof(buffer), "QUIT\n");
    write(to_server_socket, buffer, strlen(buffer));
    read(to_server_socket, buffer, sizeof(buffer) - 1);
    printf("%s", buffer);

    shutdown(to_server_socket, 2);
    close(to_server_socket);
    return EXIT_SUCCESS;
}
