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
#define PORT 30000

// Couleurs
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"

// Bataille Navale
#define WIDTH 10
#define HEIGHT 10
#define NB_BATEAUX 5

void deroute(int sig);

// Fonction pour compter les bateaux restants
int bateaux_restants(char grille[HEIGHT][WIDTH]){
    int rest = 0;
    for(int y=0;y<HEIGHT;y++)
        for(int x=0;x<WIDTH;x++)
            if(grille[y][x]=='B') rest++;
    return rest;
}

int main(void) {
    char buffer[BUFFERSIZE];
    unsigned int mon_address_longueur;
    int client_socket, ma_socket;
    int num = 0;

    struct sockaddr_in mon_address, client_address;
    struct sigaction action;

    // Grille simple
    char grille[HEIGHT][WIDTH];
    memset(grille, ' ', sizeof(grille));
    for(int i=0;i<NB_BATEAUX;i++) grille[0][i] = 'B'; // Exemple : bateaux sur la première ligne

    // Gestion des signaux
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = deroute;
    sigemptyset(&(action.sa_mask));
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGCHLD, &action, NULL);

    // Création socket
    ma_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(ma_socket == -1) { perror("socket"); exit(EXIT_FAILURE); }

    memset(&mon_address,0,sizeof(mon_address));
    mon_address.sin_port = htons(PORT);
    mon_address.sin_family = AF_INET;
    mon_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(ma_socket,(struct sockaddr *)&mon_address,sizeof(mon_address)) == -1){
        perror("bind"); exit(EXIT_FAILURE);
    }

    listen(ma_socket, 5);
    mon_address_longueur = sizeof(client_address);

    printf(GREEN "Serveur Bataille Navale prêt sur le port %d\n" RESET, PORT);

    while(1){
        client_socket = accept(ma_socket,(struct sockaddr *)&client_address,&mon_address_longueur);
        if(client_socket < 0){ perror("accept"); continue; }

        num++;

        if(fork() == 0){ // Processus fils
            close(ma_socket); // fils n’a pas besoin de la socket d’écoute
            printf("Connexion [%d] (PID=%d)\n", num, getpid());

            while(1){
                ssize_t n = read(client_socket, buffer, BUFFERSIZE-1);
                if(n <= 0) break;
                buffer[n] = '\0';

                // Affiche le message reçu côté serveur
                printf("Message reçu du client : %s", buffer);

                if(strncmp(buffer, "HELLO", 5)==0){
                    snprintf(buffer, BUFFERSIZE,"WELCOME %d %d %d\n", WIDTH, HEIGHT, NB_BATEAUX);
                    write(client_socket, buffer, strlen(buffer));
                    continue;
                }

                if(strncmp(buffer,"SHOOT",5)==0){
                    int x,y;
                    sscanf(buffer,"SHOOT %d %d",&x,&y);

                    char etat[6];
                    if(grille[y][x]=='B'){
                        grille[y][x]='H';
                        strcpy(etat,"HIT");
                        printf(YELLOW "Tir touché en (%d,%d) !\n" RESET, x, y);
                        if(bateaux_restants(grille)==0){
                            snprintf(buffer, BUFFERSIZE,"WIN %d\n", NB_BATEAUX);
                            write(client_socket, buffer, strlen(buffer));
                            printf(GREEN "Tous les bateaux sont coulés ! Partie terminée.\n" RESET);
                            break;
                        }
                    } else if(grille[y][x]=='H'){
                        strcpy(etat,"MISS"); // déjà touché, considérer comme MISS
                    } else {
                        strcpy(etat,"MISS");
                    }

                    int restants = bateaux_restants(grille);
                    snprintf(buffer, BUFFERSIZE,"RESULT %d %d %s %d\n",x,y,etat,restants);
                    write(client_socket, buffer, strlen(buffer));
                    continue;
                }

                if(strncmp(buffer,"QUIT",4)==0){
                    snprintf(buffer, BUFFERSIZE,"BYE\n");
                    write(client_socket, buffer, strlen(buffer));
                    break;
                }
            }

            shutdown(client_socket,2);
            close(client_socket);
            printf("Connexion [%d] terminée (PID=%d)\n", num, getpid());
            exit(EXIT_SUCCESS);
        }

        close(client_socket); // père ferme le socket client
    }

    shutdown(ma_socket,2);
    close(ma_socket);
    return 0;
}

void deroute(int sig){
    int c;
    if(sig==SIGCHLD){ wait(NULL); }
    else if(sig==SIGINT){
        printf(GREEN "\nSignal %s=%d reçu\n", strsignal(sig),sig);
        printf(RED "Voulez-vous réellement quitter ? [y/n]" RESET "\n");
        c=getchar(); FFLUSH_MY_STDIN();
        if(c=='y'||c=='Y'){ printf("Arrêt du serveur.\n"); exit(EXIT_SUCCESS);}
    }
}
