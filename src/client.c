// client_bataille_navale.c (Windows + Linux) - Affiche le jeu, envoie les tirs, reçoit l'état

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
  #define _WIN32_WINNT 0x0601
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET socket_t;
  #define CLOSESOCK closesocket
  #define SLEEP_MS(ms) Sleep(ms)
  #include <windows.h>
#else
  #include <unistd.h>
  #include <errno.h>
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  typedef int socket_t;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define CLOSESOCK close
  #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#define BUFSZ 512
#define DEFAULT_PORT 30000
#define GRILLE_TAILLE 5
#define NB_BATEAUX 3

// --- Fonctions de Bataille Navale (Adaptées) ---

/*
 * Rôle : Afficher l'état actuel du plateau de jeu.
 * -1 -> 'H' (case inconnue / non jouée)
 * 0  -> 'O' (tir dans l'eau)
 * 1  -> 'X' (bateau touché)
 */
void afficheplateau(int plateau[GRILLE_TAILLE][GRILLE_TAILLE])
{
    int ligne, colonne;

    printf("\n\t1 \t2 \t3 \t4 \t5"); 
    printf("\n");

    for(ligne=0 ; ligne < GRILLE_TAILLE ; ligne++ )
    {
        printf("%d",ligne+1);
        for(colonne=0 ; colonne < GRILLE_TAILLE ; colonne++ )
        {
            if(plateau[ligne][colonne]==-1)
            {
                printf("\tH");
            }
            else if(plateau[ligne][colonne]==0)
            {
                printf("\tO");
            }
            else if(plateau[ligne][colonne]==1)
            {
                printf("\tX");
            }
        }
        printf("\n");
    }
}

/*
 * Rôle : Demander au joueur 1 les coordonnées de son tir (ligne et colonne),
 * et les stocker au format chaîne "L,C".
 */
void demander_tir(char *tir_str, size_t max_len)
{
    int l, c;
    do {
        printf("\nEntrez la ligne du tir (1-5) : ");
        if (scanf("%d", &l) != 1 || l < 1 || l > 5) {
            printf("Ligne invalide.\n");
            // Nettoyer le buffer d'entrée
            int ch; while ((ch = getchar()) != '\n' && ch != EOF);
            l = 0; // Force la re-saisie
        }
    } while (l < 1 || l > 5);

    do {
        printf("Entrez la colonne du tir (1-5) : ");
        if (scanf("%d", &c) != 1 || c < 1 || c > 5) {
            printf("Colonne invalide.\n");
            // Nettoyer le buffer d'entrée
            int ch; while ((ch = getchar()) != '\n' && ch != EOF);
            c = 0; // Force la re-saisie
        }
    } while (c < 1 || c > 5);

    // Format "L,C" pour l'envoi au serveur
    snprintf(tir_str, max_len, "%d,%d", l, c);
}

#define BUFSZ 512
#define GRILLE_TAILLE 5
#define NB_BATEAUX 3

/*
 * Rôle : Désérialiser l'état du plateau et le résultat de la partie reçu du serveur.
 * Format attendu : "HHHHHHHHH...;Touches;MessageFin" (H, O, X)
 * CONVERSION : H -> -1 (Inconnu), O -> 0 (Eau), X -> 1 (Touché)
 */
void deserialiser_etat(const char *buf_in, int plateau[GRILLE_TAILLE][GRILLE_TAILLE], int *touches, char *message_fin, size_t max_msg_len) {
    
    char temp_buf[BUFSZ];
    // S'assurer que la chaîne d'entrée est manipulable et nulle-terminée
    strncpy(temp_buf, buf_in, BUFSZ - 1);
    temp_buf[BUFSZ - 1] = '\0';

    char *token;
    char *rest = temp_buf;
    
    // 1. Plateau
    token = strtok_r(rest, ";", &rest);
    int expected_len = GRILLE_TAILLE * GRILLE_TAILLE; // 25

    // Vérifie si le token existe et a la bonne longueur (25)
    if (token && (int)strlen(token) == expected_len) {
        
        int i = 0; // Index unique pour parcourir la chaîne sérialisée (token)
        for (int l = 0; l < GRILLE_TAILLE; l++) {
            for (int c = 0; c < GRILLE_TAILLE; c++) {
                
                char state_char = token[i];
                
                // Conversion basée sur les caractères envoyés par le serveur
                if (state_char == 'H')      { plateau[l][c] = -1; }
                else if (state_char == 'O') { plateau[l][c] = 0; }
                else if (state_char == 'X') { plateau[l][c] = 1; }
                else {
                    // Cas d'erreur dans la sérialisation si un caractère inconnu est reçu
                    plateau[l][c] = -2; 
                    printf("\nCaractère inconnu reçu (%c). Erreur de sérialisation.\n", state_char);
                }
                
                i++; // Avancer l'index de la chaîne sérialisée
            }
        }
    } else {
        printf("\nErreur de format de plateau. Longueur inattendue (%d/%d).\n", 
               token ? (int)strlen(token) : 0, expected_len);
        return;
    }

    // 2. Touches
    token = strtok_r(NULL, ";", &rest);
    if (token) {
        *touches = atoi(token);
    } 

    // 3. Message de Fin de Partie
    token = strtok_r(NULL, ";", &rest);
    if (token) {
        strncpy(message_fin, token, max_msg_len - 1);
        message_fin[max_msg_len - 1] = '\0';
    } else {
        message_fin[0] = '\0'; 
    }
}

// --- Fonctions Réseau (Portable) ---

static void die(const char *msg) {
#ifdef _WIN32
  fprintf(stderr, "%s (WSA=%d)\n", msg, WSAGetLastError());
#else
  perror(msg);
#endif
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    const char *ip = (argc >= 2) ? argv[1] : "127.0.0.1";
    int port = (argc >= 3) ? atoi(argv[2]) : DEFAULT_PORT;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) die("WSAStartup");
#endif

    socket_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) die("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "IP invalide: %s\n", ip);
        exit(EXIT_FAILURE);
    }

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) die("connect");

    printf("Client connecté à %s:%d\n", ip, port);
    printf ("\t******************************\n\t****Projet bataille navale****\n\t******************************");

    // Variables de partie
    int plateau[GRILLE_TAILLE][GRILLE_TAILLE];
    int touches = 0;
    char buf[BUFSZ];
    char tir_str[10];
    char message_fin[100];
    
    // 1. Première réception: état initial du plateau
    int r = recv(s, buf, BUFSZ - 1, 0);
    if (r == 0) { printf("Serveur fermé.\n"); CLOSESOCK(s); return 0; }
    if (r == SOCKET_ERROR) die("recv init");
    buf[r] = '\0';
    
    // Désérialiser et afficher l'état initial
    deserialiser_etat(buf, plateau, &touches, message_fin, sizeof(message_fin));
    afficheplateau(plateau);

    // Boucle de jeu
    while(touches != NB_BATEAUX) {
        
        // 2. Demander et envoyer le tir
        demander_tir(tir_str, sizeof(tir_str));
        
        int n = send(s, tir_str, (int)strlen(tir_str), 0);
        if (n == SOCKET_ERROR) die("send");
        
        printf("Envoi du tir: %s\n", tir_str);

        // 3. Recevoir l'état mis à jour
        r = recv(s, buf, BUFSZ - 1, 0);
        if (r == 0) { printf("Serveur fermé.\n"); break; }
        if (r == SOCKET_ERROR) die("recv");
        buf[r] = '\0';
        
        // 4. Désérialiser et afficher le nouvel état
        deserialiser_etat(buf, plateau, &touches, message_fin, sizeof(message_fin));
        
        // Affichage du plateau mis à jour
        afficheplateau(plateau);
        
        // Affichage du nombre de touches
        printf("\nNavires touchés: %d / %d\n", touches, NB_BATEAUX);
        
        // Vérification du message de fin de partie
        if (touches == NB_BATEAUX) {
            printf("\n%s\n", message_fin);
        } else {
             // Pour éviter de saturer la CPU pendant la réflexion du joueur si ce n'était pas interactif
             // Mais dans ce cas interactif, on ne fait pas de pause explicite
        }
    }

    CLOSESOCK(s);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
