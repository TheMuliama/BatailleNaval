#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h> // Pour strlen, memset, etc.

#define PORT 30000
#define BUFFERSIZE 512
#define RESET "\033[0m"
#define MAGENTA "\033[35m"

int main()
{
    int to_server_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFERSIZE];

    // Création de la socket
    to_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (to_server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l’adresse du serveur
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connexion au serveur
    if (connect(to_server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur sur le port %d\n", PORT);

    // Boucle principale d’échange
    while (1) {
        const char *msg = "Voilà un code très secret qui transite [biloute]";
        ssize_t n = write(to_server_socket, msg, strlen(msg)); // ← longueur réelle du message
        if (n == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        sleep(1);

        ssize_t r = read(to_server_socket, buffer, BUFFERSIZE - 1);
        if (r == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (r == 0) {
            printf("Serveur fermé.\n");
            break;
        }

        buffer[r] = '\0'; // Fin de chaîne
        printf("... le client a reçu : %s%s%s\n", MAGENTA, buffer, RESET);
    }

    close(to_server_socket);
    return 0;
}

