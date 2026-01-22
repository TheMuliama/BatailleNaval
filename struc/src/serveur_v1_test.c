#define MAX_SIZE 20 // Taille max pour éviter l'allocation dynamique complexe
typedef struct {
    int plateau[MAX_SIZE][MAX_SIZE]; // -1: inconnu/eau, 0: raté, 1: touché/coulé
    int bateaux[3][2]; // 3 navires de taille 1
    int n; // Taille de la grille (ex: 5)
    int nb_coules;
} GameState;

// ... (Includes standards: stdio, stdlib, string, sys/socket, netinet/in, unistd, signal, time) ...

#define PORT 30000
#define BUFFERSIZE 512

void init_jeu(int plateau[5][5], int bateaux[3][2]) {
    srand(time(NULL));
    // Init plateau vide
    for(int i=0; i<5; i++) for(int j=0; j<5; j++) plateau[i][j] = -1;
    // Init bateaux (code repris de exemple_simple.c)
    for(int bat=0; bat<3; bat++) {
        bateaux[bat][0] = rand()%5;
        bateaux[bat][1] = rand()%5;
        // Vérif doublons simple... (omis pour brièveté, voir votre fichier)
    }
}

int main() {
    // ... (Setup socket identique à serveur.c: socket, bind, listen) ...
    int ma_socket; // (Setup et bind ici...)
    // ...
    
    while(1) {
        struct sockaddr_in client_address;
        unsigned int len = sizeof(client_address);
        int client_socket = accept(ma_socket, (struct sockaddr *)&client_address, &len);
        
        if (fork() == 0) { // Gestion multi-client native (V3 ready)
            close(ma_socket);
            
            // Initialisation du jeu pour ce client
            int plateau[5][5];
            int bateaux[3][2];
            int nb_touches = 0;
            init_jeu(plateau, bateaux);
            
            char buffer[BUFFERSIZE];
            while(1) {
                // 1. Affichage Flotte Serveur (En clair)
                system("clear");
                printf("=== FLOTTE SERVEUR (CLIENT %d) ===\n", getpid());
                for(int i=0; i<5; i++) {
                    for(int j=0; j<5; j++) {
                        int is_bat = 0;
                        for(int k=0; k<3; k++) if(bateaux[k][0]==i && bateaux[k][1]==j) is_bat=1;
                        
                        if(plateau[i][j] == 1) printf(" X ");
                        else if(is_bat) printf(" B "); // Bateau visible
                        else printf(" ~ ");
                    }
                    printf("\n");
                }

                // 2. Réception du tir (Format "LIGNE COLONNE")
                memset(buffer, 0, BUFFERSIZE);
                if (read(client_socket, buffer, BUFFERSIZE) <= 0) break;
                
                int l, c;
                sscanf(buffer, "%d %d", &l, &c);
                l--; c--; // Ajustement index 0

                // 3. Traitement du tir
                char reponse[BUFFERSIZE];
                int touche = 0;
                for(int k=0; k<3; k++) {
                    if(bateaux[k][0] == l && bateaux[k][1] == c) {
                        touche = 1;
                    }
                }

                if(touche) {
                    plateau[l][c] = 1;
                    nb_touches++;
                    if(nb_touches >= 3) sprintf(reponse, "GAGNE");
                    else sprintf(reponse, "COULE"); // Navire taille 1 = Coulé direct
                } else {
                    plateau[l][c] = 0;
                    sprintf(reponse, "RATE");
                }

                // 4. Envoi réponse
                write(client_socket, reponse, strlen(reponse)+1);
                if(nb_touches >= 3) break; // Fin de partie
            }
            close(client_socket);
            exit(0);
        }
        close(client_socket);
    }
    return 0;
}
