# ZomboGIG

Jeu de survie isométrique contre des zombies, réalisé avec Qt 6 Widgets. Tout
est dessiné par Qt : aucun asset externe, aucune bibliothèque 3D.

## Le jeu

- Carte isométrique de 12 x 12 avec un camp entouré de murs, deux portes, deux
  maisons, un feu de camp, des plants de nourriture et quelques décorations.
- Un tour = une heure, la partie commence à 07:00 sous une pluie légère. Il n'y
  a pas de limite de temps : tant qu'il reste des survivants et des zombies, la
  partie continue.
- Le menu principal permet de choisir : le nombre de survivants (1 à 12), le
  nombre de zombies (1 à 30), les plants de départ (0 à 6), la solidité des murs
  (4 à 20), les métiers humains autorisés, les types de zombies autorisés et une
  graine aléatoire.
- Les humains (Farmer, Doctor, Military, Builder) et les zombies (Tank, Runner,
  Exploder) agissent automatiquement à chaque tour. Les humains mangent,
  récoltent, soignent, réparent et combattent ; les zombies poursuivent,
  frappent les murs, les portes et les maisons, et explosent.
- Les portes restent fermées pendant l'attaque des zombies. Avant d'agir, les
  survivants ouvrent les portes adjacentes pour circuler ; les zombies peuvent
  traverser une porte ouverte ou la briser.
- Une nouvelle vague arrive toutes les 8 heures et devient plus nombreuse avec
  le temps.
- Les plants repoussent chaque tour (la pluie double la croissance). La faim
  baisse de 15 points par tour : un survivant affamé (50 ou moins) prend une
  unité dans la réserve commune et remonte à 100.
- Tous les 6 tours, si le camp compte au moins 3 survivants, un enfant rejoint
  le groupe ou un survivant adulte change de métier. Un enfant devient adulte au
  bout de 6 tours.
- Un survivant infecté se transforme en zombie au bout de 4 tours s'il n'est pas
  soigné par un docteur.

## Conditions de fin de partie

La partie s'arrête à la fin d'un tour, dans l'un de ces trois cas :

1. **Défaite — plus aucun survivant.** Dès que la liste des survivants est vide
   (tués par les zombies ou transformés après infection), l'état passe à
   `GameOver` avec `victoire_humains = false`. Le message affiché est
   « THE CAMP HAS FALLEN ».
2. **Victoire — plus aucun zombie.** Si au moins un survivant est encore en vie
   et que la horde est vide, l'état passe à `GameOver` avec
   `victoire_humains = true`. Le message affiché est « THE HORDE IS GONE ».
3. **Défaite — réserve de nourriture vide.** Si `stock_nourriture` tombe à 0,
   c'est la famine et la partie est perdue.

Points importants :

- La défaite par survivants est vérifiée avant la victoire, puis la famine.
- Les survivants infectés mais pas encore transformés comptent encore comme
  vivants ; ils ne sont retirés qu'à leur transformation (4 tours d'infection).
- Des vagues continuent d'arriver tant que la partie dure : pour gagner, il faut
  détruire la horde avant la vague suivante.

## Contrôles

- `Espace` — lancer / mettre en pause le déroulement automatique
- `N` ou `→` — avancer d'une heure
- `1` `2` `3` `4` — vitesse du déroulement automatique
- `Échap` — retour au menu
- `F11` — plein écran
- **Survoler une case de la carte affiche ses statistiques** (type, points de
  vie, murs, plants, décorations).

## Interface

- Pluie animée tant que le temps est pluvieux, puis éclaircie.
- Éclairage jour/nuit, lueur du feu de camp, brouillard dérivant et taches de
  sang.
- HUD et menu au style « survie » usé, avec un court texte d'introduction.
- Interface adaptative : la fenêtre s'ouvre maximisée et le HUD suit la hauteur.
- Police optionnelle : placez un fichier `.ttf`/`.otf` dans un dossier `fonts/`
  à côté de l'exécutable (ou à la racine du projet), il sera chargé
  automatiquement.

## Organisation du projet

Le code est rangé par rôle pour rester simple à lire et à présenter. Il n'y a
pas de fichiers d'en-tête : chaque module contient ses déclarations puis son
code, et `main.cpp` les assemble dans l'ordre.

- `src/main.cpp` — le cœur du jeu : la classe `Game`, les énumérations
  `GameState` et `Weather`, les tours, les portes, les vagues, la météo, les
  apparitions, l'accueil de nouveaux survivants et la fin de partie.
- `src/logic/world.cpp` — le plateau : `Wall` (mur, porte ou maison), `Plant`,
  `Cell` et `Grid` (occupation des cases, murs, plantes, décorations).
- `src/entities/human.cpp` — `Entity` (base commune), `Human`, l'énumération
  `HumanType` (`Farmer`, `Doctor`, `Military`, `Builder`, `Child`), leurs
  statistiques, leurs capacités et les déplacements communs.
- `src/entities/zombie.cpp` — `Zombie`, l'énumération `ZombieType`, les
  statistiques et capacités de `Tank`, `Runner` et `Exploder`.
- `src/ui/ui.cpp` — les widgets, le rendu de la carte, le menu et le HUD.
- Fabriques : `creer_humain` et `creer_zombie`.

Classes du modèle : `Game`, `GameState`, `Grid`, `Cell`, `Entity` (abstraite),
`Human`, `Zombie`, `Plant`, `Wall`, `Status`.

## À propos du code et de l'IA

L'interface et le design visuel dans `src/ui/` ont été réalisés avec l'aide d'un
assistant IA. La simulation elle-même — tout `src/logic/` et `src/entities/`,
c'est-à-dire les règles du jeu et les classes d'entités — a été écrite par
l'étudiant sans IA et peut être expliquée ligne par ligne.

Les noms dans `src/logic/` et `src/entities/` sont volontairement descriptifs
(par exemple `survivant_le_plus_proche`, `mur_abime_a_cote`,
`durabilite_initiale_murs`) et le code est aéré (accolades Allman, une idée par
ligne) pour être lu et présenté en cours.

## Compilation (WSL / Ubuntu)

```sh
make
```

Le Makefile lance une compilation dans `build/` via `qmake6`.

## Lancer le jeu

```sh
make run
```

ou :

```sh
./build/ZomboGIG
```

## Nettoyer

```sh
make clean
```

## Prérequis

- g++ avec C++20
- make
- qt6-base-dev, qt6-base-dev-tools (Qt 6.10.2)
- WSLg pour la fenêtre (WSL 2 sous Windows 11)
