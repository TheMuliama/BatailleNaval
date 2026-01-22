// serveur_bataille_navale.c (Windows + Linux) - Gère la logique du jeu et un seul client (portable)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Pour srand/time

#ifdef _WIN32
  #define _WIN32_WINNT 0x0601
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET socket_t;
  #define CLOSESOCK closesocket
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
#endif

#define BUFSZ 512
#define DEFAULT_PORT 30001
#define GRILLE_TAILLE 5
#define NB_BATEAUX 3

// --- Fonctions de Bataille Navale ---

/*
 * Rôle : Initialiser le plateau de jeu à l'état "vide" (-1).
 */
void initplateau(int plateau[GRILLE_TAILLE][GRILLE_TAILLE])
{
    int ligne, colonne;
    for(ligne=0 ; ligne < GRILLE_TAILLE ; ligne++ )
        for(colonne=0 ; colonne < GRILLE_TAILLE ; colonne++ )
            plateau[ligne][colonne]=-1;
}

/*
 * Rôle : Placer aléatoirement les bateaux (s'assurer qu'ils ne se superposent pas).
 */
void initbateau(int bateaux[NB_BATEAUX][2])
{
    srand(time(NULL));
    int bat, dern;
    int est_valide;

    for(bat=0 ; bat < NB_BATEAUX ; bat++)
    {
        do {
            est_valide = 1;
            bateaux[bat][0] = rand() % GRILLE_TAILLE; // Ligne (0-4)
            bateaux[bat][1] = rand() % GRILLE_TAILLE; // Colonne (0-4)

            // Vérifier la superposition avec les bateaux précédents
            for(dern=0 ; dern < bat ; dern++)
            {
                if((bateaux[bat][0] == bateaux[dern][0]) && (bateaux[bat][1] == bateaux[dern][1]))
                {
                    est_valide = 0; // Superposition trouvée, on re-tire
                    break;
                }
            }
        } while(!est_valide);
    }
}

/*
 * Rôle : Vérifier si le tir correspond à la position d'un des bateaux.
 * Retour : 1 si un bateau est touché, 0 sinon.
 */
int touche (int tir[2], int bateaux[NB_BATEAUX][2])
{
    int bat;
    for(bat=0 ; bat < NB_BATEAUX ; bat++)
    {
        if( tir[0]==bateaux[bat][0] && tir[1]==bateaux[bat][1])
        {
            return 1;
        }
    }
    return 0;
}

/*
 * Rôle : Mettre à jour le plateau en fonction du résultat du tir.
 */
void changeplateau(int tir[2], int bateaux[NB_BATEAUX][2], int plateau[GRILLE_TAILLE][GRILLE_TAILLE])
{
    if(touche(tir, bateaux))
        plateau[tir[0]][tir[1]]=1; // Touché (X)
    else
        plateau[tir[0]][tir[1]]=0; // Dans l'eau (O)
}

/*
 * Rôle : Afficher l'état des bateaux (pour le serveur)
 */
void afficher_flotte(int bateaux[NB_BATEAUX][2])
{
    printf("--- Flotte (Positions Claires) ---\n");
    for(int i = 0; i < NB_BATEAUX; i++) {
        printf("Bateau %d: L%d C%d\n", i + 1, bateaux[i][0] + 1, bateaux[i][1] + 1);
    }
    printf("----------------------------------\n");
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

/*
 * Rôle : Sérialiser l'état du plateau et le résultat de la partie.
 * Format : "HHHHHHHHH...;Touches;MessageFin" (H, O, X)
 */
void serialiser_etat(int plateau[GRILLE_TAILLE][GRILLE_TAILLE], int touches, int essais, char *buf_out, size_t buf_len) {
    char plateau_str[50]; // 5x5 = 25 chars
    plateau_str[0] = '\0';
    
    for (int l = 0; l < GRILLE_TAILLE; l++) {
        for (int c = 0; c < GRILLE_TAILLE; c++) {
            char c_state[2] = "";
            
            // CONVERSION: -1 -> H, 0 -> O, 1 -> X (1 caractère fixe)
            if (plateau[l][c] == -1)      { c_state[0] = 'H'; }
            else if (plateau[l][c] == 0)  { c_state[0] = 'O'; }
            else if (plateau[l][c] == 1)  { c_state[0] = 'X'; }
            else                          { c_state[0] = '?'; } // Erreur

            strcat(plateau_str, c_state);
        }
    }

    char fin_partie[100] = "";
    if (touches == NB_BATEAUX) {
        snprintf(fin_partie, 100, "****GAGNE***, vous avez triomphe de l'ennemi en %d tirs", essais);
    }

    // Sérialisation finale
    snprintf(buf_out, buf_len, "%s;%d;%s", plateau_str, touches, fin_partie);
}

int main(int argc, char **argv) {
    int port = (argc >= 2) ? atoi(argv[1]) : DEFAULT_PORT;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) die("WSAStartup");
#endif

    socket_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == INVALID_SOCKET) die("socket");

#ifndef _WIN32
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) die("bind");
    if (listen(srv, 5) == SOCKET_ERROR) die("listen");

    printf("Serveur en écoute sur le port %d\n", port);

    // Initialisation du jeu pour le serveur
    int plateau[GRILLE_TAILLE][GRILLE_TAILLE];
    int bateaux[NB_BATEAUX][2];
    initplateau(plateau);
    initbateau(bateaux);

    printf ("\t******************************\n\t****Projet bataille navale****\n\t******************************\n");
    afficher_flotte(bateaux); // Affiche la flotte en clair (V1)
    
    // Attente et gestion d'une seule connexion client
    struct sockaddr_in cli;
    socklen_t clen = (socklen_t)sizeof(cli);
    
    // CORRECTION APPLIQUÉE ICI : Déclaration explicite de 'c' (socket_t)
    socket_t c = accept(srv, (struct sockaddr*)&cli, &clen);
    if (c == INVALID_SOCKET) die("accept");

    char ip[64];
    inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
    printf("Client connecté: %s:%d\n", ip, ntohs(cli.sin_port));

    // Variables de partie
    int touches = 0;
    int essais = 0;
    char buf[BUFSZ];
    char rep[BUFSZ];

    // Première réponse: état initial du plateau
    serialiser_etat(plateau, touches, essais, rep, sizeof(rep));
    if (send(c, rep, (int)strlen(rep), 0) == SOCKET_ERROR) die("send init");

    while(touches < NB_BATEAUX) {
      int r = recv(c, buf, BUFSZ - 1, 0);
      if (r == 0) { printf("Client déconnecté.\n"); break; }
      if (r == SOCKET_ERROR) die("recv");

      buf[r] = '\0'; // Le buf contient le tir "L,C" (ex: "2,3")
      printf("Reçu tir du client: %s\n", buf);

      // 1. Parser le tir L,C
      int tir[2];
      if (sscanf(buf, "%d,%d", &tir[0], &tir[1]) != 2) {
          printf("Tir invalide reçu.\n");
          continue; 
      }
      
      // Convertir coordonnées 1-5 en indices 0-4
      tir[0]--;
      tir[1]--;

      // Vérifier si le tir est hors limites ou déjà joué
      if (tir[0] < 0 || tir[0] >= GRILLE_TAILLE || tir[1] < 0 || tir[1] >= GRILLE_TAILLE || plateau[tir[0]][tir[1]] != -1) {
          printf("Tir hors limites ou case déjà jouée, ignoré.\n");
          // Renvoyer l'état actuel pour que le client ne se bloque pas
          serialiser_etat(plateau, touches, essais, rep, sizeof(rep));
          if (send(c, rep, (int)strlen(rep), 0) == SOCKET_ERROR) die("send error");
          continue; 
      }
      
      essais++; // Incrémenter le nombre d'essais (seulement si le tir est valide)

      // 2. Traiter le tir
      if (touche(tir, bateaux)) {
          printf("****TOUCHE**** (%d,%d)\n", tir[0] + 1, tir[1] + 1);
          touches++;
      }
      
      // 3. Mettre à jour le plateau
      changeplateau(tir, bateaux, plateau);
      
      // 4. Sérialiser l'état et répondre au client
      serialiser_etat(plateau, touches, essais, rep, sizeof(rep));
      int n = send(c, rep, (int)strlen(rep), 0);
      if (n == SOCKET_ERROR) die("send");
    }

    CLOSESOCK(c);
    CLOSESOCK(srv);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
