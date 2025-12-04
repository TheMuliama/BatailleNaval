#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>                     
#include <sys/socket.h>
#include <netinet/in.h>       
#include <arpa/inet.h>

#define FFLUSH_MY_STDIN() {int _c_; while ((_c_ = getchar()) != '\n' && _c_ != EOF){}}
#define BUFFERSIZE 512
#define PORT 30001

// COULEURS
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"

void deroute(int sig);

int main(void)
{
    const char *reponse = "Reçu";
    char buffer[BUFFERSIZE];
    unsigned int mon_address_longueur;
    int client_socket, ma_socket;
    int num = 0;

    struct sockaddr_in mon_address, client_address;
    struct sigaction action;  

    memset(&action, 0, sizeof(action));  
    action.sa_handler = deroute;
    sigemptyset(&(action.sa_mask));
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGCHLD, &action, NULL);

    memset(&mon_address, 0, sizeof(mon_address));
    mon_address.sin_port = htons(PORT);
    mon_address.sin_family = AF_INET;
    mon_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((ma_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(ma_socket, (struct sockaddr *)&mon_address, sizeof(mon_address)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(ma_socket, 5);
    mon_address_longueur = sizeof(client_address);

    while (1) {
        client_socket = accept(ma_socket, (struct sockaddr *)&client_address, &mon_address_longueur);
        num++;

        if (fork() == 0) {
            close(ma_socket);
            printf("\nConnexion [%d] avec le client (PID=%d)\n", num, getpid());

            while (1) {
                ssize_t r = read(client_socket, buffer, BUFFERSIZE - 1);
                if (r <= 0) {
                    if (r < 0) perror("read");
                    break;
                }
                buffer[r] = '\0';
                printf("... le serveur a reçu : %s%s%s\n", RED, buffer, RESET);

                sleep(1);

                ssize_t n = write(client_socket, reponse, strlen(reponse)); // ✅ longueur réelle
                if (n <= 0) {
                    if (n < 0) perror("write");
                    break;
                }
            }

            shutdown(client_socket, 2);
            close(client_socket);
            printf("Terminaison connexion [%d] (PID=%d)\n", num, getpid());
            return EXIT_SUCCESS;
        }
    }

    shutdown(ma_socket, 2);
    close(ma_socket);
    return EXIT_SUCCESS;
}

void deroute(int sig)
{
    if (sig == SIGCHLD) {
        printf("Terminaison du processus fils : %d\n", wait(NULL));
        exit(EXIT_SUCCESS); 
    } else if (sig == SIGINT) {
        printf(GREEN "\tSignal %d reçu — arrêt du serveur\n" RESET, sig);
        exit(EXIT_SUCCESS);
    }
}
