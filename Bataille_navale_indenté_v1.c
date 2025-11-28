#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/*
 * Fonction : initplateau
 * Rôle     : Initialiser le plateau de jeu à l'état "vide".
 *            Chaque case est mise à -1 pour indiquer qu'aucun tir n'a encore été effectué.
 * Paramètres :
 *   - plateau : tableau 5x5 représentant la grille de jeu.
 * Retour : aucun.
 */
void initplateau(int plateau[][5])          // initialise le plateau
{
    int ligne, colonne;
    for(ligne=0 ; ligne < 5 ; ligne++ )     //  0 1 2 3 4
        for(colonne=0 ; colonne < 5 ; colonne++ ) // 0 1 2 3 4
            plateau[ligne][colonne]=-1;
}

/*
 * Fonction : afficheplateau
 * Rôle     : Afficher à l'écran l'état actuel du plateau de jeu.
 *            -1 -> 'H' (case inconnue / non jouée)
 *             0 -> 'O' (tir dans l'eau)
 *             1 -> 'X' (bateau touché)
 * Paramètres :
 *   - plateau : tableau 5x5 représentant la grille de jeu.
 * Retour : aucun.
 */
void afficheplateau(int plateau[][5])
{
    int ligne, colonne;

    printf("\t1 \t2 \t3 \t4 \t5"); // 1 2 3 4 5
    printf("\n");

    for(ligne=0 ; ligne < 5 ; ligne++ )   // for l 0 1 2 3 4
    {
        printf("%d",ligne+1);
        for(colonne=0 ; colonne < 5 ; colonne++ )
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
 * Fonction : initbateau
 * Rôle     : Placer aléatoirement 3 bateaux sur la grille.
 *            Chaque bateau est défini par ses coordonnées (ligne, colonne).
 *            La fonction s'assure qu'aucun bateau ne se superpose.
 * Paramètres :
 *   - bateaux : tableau 3x2, chaque ligne contient [ligne, colonne] d'un bateau.
 * Retour : aucun.
 */
void initbateau(int bateaux[][2])
{
    srand(time(NULL));                  // initialise le random
    int bat, dern;

    for(bat=0 ; bat < 3 ; bat++)
    {
        bateaux[bat][0]= rand()%5;
        bateaux[bat][1]= rand()%5;

        /* on vérifie que le tir n'a pas déjà été fait, si le tir est fait on sort  
         * de la boucle dowhile, si pas déjà fait ça marche */
        for(dern=0 ; dern < bat ; dern++)  //dern 0 1 2
        {
            if((bateaux[bat][0] == bateaux[dern][0])&&(bateaux[bat][1] == bateaux[bat][1]))
                do
                {
                    bateaux[bat][0]= rand()%5;
                    bateaux[bat][1]= rand()%5;
                }
                while((bateaux[bat][0] == bateaux[dern][0])&&(bateaux[bat][1] == bateaux[dern][1]));
        }
    }
}

/*
 * Fonction : tir_j1
 * Rôle     : Demander au joueur 1 les coordonnées de son tir (ligne et colonne),
 *            puis les convertir en indices de tableau (0 à 4).
 * Paramètres :
 *   - tir : tableau de 2 entiers, tir[0] = ligne, tir[1] = colonne.
 * Retour : aucun.
 */
void tir_j1(int tir[2])
{
    printf("\nEntrez la ligne du tir : ");
    scanf("%d",&tir[0]);
    tir[0]--;

    printf("\nEntre la colonne du tir : ");
    scanf("%d",&tir[1]);
    tir[1]--;
}

/*
 * Fonction : touche
 * Rôle     : Vérifier si le tir du joueur correspond à la position d'un des bateaux.
 * Paramètres :
 *   - tir      : tableau [ligne, colonne] du tir.
 *   - bateaux  : tableau 3x2 des positions des bateaux.
 * Retour :
 *   - 1 si un bateau est touché,
 *   - 0 sinon.
 */
int touche (int tir[2], int bateaux[][2])
{
    int bat;

    for(bat=0 ; bat < 3 ; bat++)
    {
        if( tir[0]==bateaux[bat][0] && tir[1]==bateaux[bat][1])
        {
            printf("****TOUCHE**** (%d,%d)\n",tir[0]+1,tir[1]+1);
            return 1;
        }
    }
    return 0;
}

/*
 * Fonction : com
 * Rôle     : Donner un indice au joueur en comptant :
 *            - le nombre de bateaux sur la même ligne que le tir,
 *            - le nombre de bateaux sur la même colonne que le tir.
 * Paramètres :
 *   - tir      : tableau [ligne, colonne] du tir.
 *   - bateaux  : tableau 3x2 des positions des bateaux.
 *   - essai    : numéro de l'essai (tir courant).
 * Retour : aucun.
 */
void com(int tir[2], int bateaux[][2], int essai)
{
    int ligne=0,colonne=0,rang;

    // compte le nombre de bateaux ligne/colonne
    for(rang=0 ; rang < 3 ; rang++)
    {
        if(bateaux[rang][0]==tir[0])
            ligne++;
        if(bateaux[rang][1]==tir[1])
            colonne++;
    }

    printf("\nessai %d: \nligne %d -> %d bateaux\ncolonne %d -> %d bateaux\n",essai,tir[0]+1,ligne,tir[1]+1,colonne);
}

/*
 * Fonction : changeplateau
 * Rôle     : Mettre à jour le plateau en fonction du résultat du tir :
 *            - 1 si un bateau est touché,
 *            - 0 si c'est dans l'eau.
 * Paramètres :
 *   - tir      : tableau [ligne, colonne] du tir.
 *   - bateaux  : tableau 3x2 des positions des bateaux.
 *   - plateau  : tableau 5x5 représentant la grille de jeu.
 * Retour : aucun.
 */
void changeplateau(int tir[2], int bateaux[][2], int plateau[][5])
{
    if(touche(tir,bateaux))
        plateau[tir[0]][tir[1]]=1;
    else
        plateau[tir[0]][tir[1]]=0;
}

/*
 * Fonction : main
 * Rôle     : Point d'entrée du programme.
 *            - Initialise le plateau et les bateaux.
 *            - Boucle tant que les 3 bateaux n'ont pas été touchés.
 *            - À chaque tour :
 *                 * affiche le plateau,
 *                 * lit un tir,
 *                 * indique si un bateau est touché,
 *                 * donne des indices (com),
 *                 * met à jour le plateau.
 *            - Affiche un message de victoire à la fin.
 * Paramètres : aucun.
 * Retour :
 *   - 0 (fin normale du programme).
 */
int main()
{
    int plateau[5][5];
    int bateaux[3][2];
    int tir[2];
    int essais=0,touches=0;         // le jeu n'a pas commencé
    printf ("\t******************************\n\t****Projet bataille navale****\n\t******************************");
    initplateau(plateau);           // initialise le plateau
    initbateau(bateaux);            // initialise les bateaux
    printf("\n");

    do
    {
        afficheplateau(plateau);    //
        tir_j1(tir);                // fonction de jeu ...
        essais++;                   // à chaque essai, essais augmente de 1

        if( touche (tir,bateaux))
        {
            com(tir,bateaux,essais);
            touches++;
        }
        else
            com(tir,bateaux,essais);

        changeplateau(tir,bateaux,plateau);


    }
    while(touches!=3);  // !=

    printf("\n\n\n****GAGNE***, vous avez triomphe de l'ennemi en %d tirs", essais);
    afficheplateau(plateau);
}
