#define MAX_SIZE 20 // Taille max pour éviter l'allocation dynamique complexe
typedef struct {
    int plateau[MAX_SIZE][MAX_SIZE]; // -1: inconnu/eau, 0: raté, 1: touché/coulé
    int bateaux[3][2]; // 3 navires de taille 1
    int n; // Taille de la grille (ex: 5)
    int nb_coules;
} GameState;

// ... (Includes standards et setup socket identique à client.c) ...

int main() {
    // ... (Connexion au serveur réussie sur 'to_server_socket') ...
    
    int mon_plateau[5][5];
    // Init plateau local à 'H' (inconnu)
    for(int i=0; i<5; i++) for(int j=0; j<5; j++) mon_plateau[i][j] = -1;

    char buffer[BUFFERSIZE];
    int fin = 0;

    while(!fin) {
        // 1. Affichage permanent
        system("clear");
        printf("\t1 \t2 \t3 \t4 \t5\n");
        for(int i=0; i<5; i++) {
            printf("%d", i+1);
            for(int j=0; j<5; j++) {
                if(mon_plateau[i][j] == -1) printf("\tH"); // Brouillard
                else if(mon_plateau[i][j] == 0) printf("\t0"); // Raté
                else if(mon_plateau[i][j] == 1) printf("\tX"); // Touché
            }
            printf("\n");
        }

        // 2. Saisie Tir
        int l, c;
        printf("\nTir (Ligne Colonne) : ");
        scanf("%d %d", &l, &c);
        
        // 3. Envoi au serveur
        sprintf(buffer, "%d %d", l, c);
        write(to_server_socket, buffer, strlen(buffer)+1);

        // 4. Réception Résultat
        memset(buffer, 0, BUFFERSIZE);
        read(to_server_socket, buffer, BUFFERSIZE);

        // 5. Mise à jour locale
        if(strcmp(buffer, "COULE") == 0 || strcmp(buffer, "GAGNE") == 0) {
            mon_plateau[l-1][c-1] = 1; // X
            printf("--> NAVIRE COULE !\n");
        } else {
            mon_plateau[l-1][c-1] = 0; // 0
            printf("--> A L'EAU...\n");
        }

        if(strcmp(buffer, "GAGNE") == 0) {
            printf("\n*** VICTOIRE : TOUTE LA FLOTTE EST COULEE ***\n");
            fin = 1;
        }
        sleep(1); // Petit délai pour lire le message
    }
    close(to_server_socket);
    return 0;
}
