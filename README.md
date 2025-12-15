# Bataille Naval

Ce projet à pour objectif de dévelloper une application permettant de pouvoir jouer au bataille naval.


## Contexte

Il s'incrit dans le contexte d'une SAE et à pour but de réaliser un service utilisant le modèle de client-serveur. Plus précisément réaliser un jeu de bataille naval.
Il vise à réaliser une application à l'aide du language de programmation C.

Ce projet doit être réalisé avant fin Janvier 2026. 

## Fonctionnalités attendus V1

- [x] Mettre en place CMake (par pitié)

**Code client**

- [ ] Charger les données des tirs précédents depuis le serveur
- [ ] Envoyer les inputs de l'utilisateur au serveur
- [ ] Laisser le client le choix du serveur (Choix de l'addresse IP et du PORT)
- [ ] Afficher les résultats sous forme d'un interface dans le CLI.
- [ ] L'interface doit être esthétique et pratique pour l'utilisateur.

**Code serveur**
- [ ] Initialisation de chaque nouvelle partie
- [ ] Stocker les informations du plateau
- [ ] Recevoir les input d'une client et mettre à jour le plateau
- [ ] Afficher une vue complète de toute la partie
- [ ] Envoyer les résultats des tirs de l'utilisateur au client

## Achitecture du projet 

- `src/` : Code source
- `build/` : Contient les exécutables
- `data/` : Données d’entrée du programme (fichier de configuration, données de test etc.)
- `tests/` : Code tests
- `CMakesFiles` : Fichier pour la compilation
- `README.md` : Documentation du projet
