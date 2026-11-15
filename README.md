# ZomboGIG

ZomboGIG est un jeu de survie contre des zombies. Les survivants et les zombies
agissent automatiquement à chaque tour : gérez les ressources, protégez la base
et survivez aux vagues.

Le projet est inspiré du jeu *Project Zomboid* et de son thème de survie face
aux zombies.

## Comment jouer

- La partie commence à 07 h. Un tour représente une heure.
- La carte fait 12 x 12 cases. La base a un périmètre rectangulaire compact
  avec deux portes et deux maisons à l'intérieur. Les survivants peuvent sortir
  par les portes pour rejoindre les zombies ou explorer la carte.
- Les portes s'ouvrent lorsqu'un survivant se trouve à côté. Elles se referment
  au tour suivant ; les zombies peuvent les attaquer.
- Le nombre de zombies choisi dans le menu apparait au debut de la partie.
  Ensuite, une vague arrive sur les bords de la carte toutes les 8 heures :
  les deux premieres ajoutent 3 zombies chacune, puis chaque vague en ajoute
  5. Les types de zombies viennent de la selection du menu.
- Les plantes poussent à chaque tour, plus vite quand il pleut. Les fermiers
  récoltent les plantes mûres et ajoutent la nourriture à la réserve commune.
- Certains plants se trouvent à l'extérieur de la base : les fermiers peuvent
  passer par les portes pour aller les récolter.
- Des colis de ravitaillement contenant 3 à 5 rations tombent parfois dehors.
  Un survivant les ramasse automatiquement en marchant sur la case.
- Les zombies apparaissent à l'extérieur du périmètre, pas au milieu de la base.
- Un zombie éliminé laisse un cadavre visible pendant 6 tours ; il s'efface
  progressivement.
- La faim diminue au fil des tours. Les survivants mangent dans la réserve ;
  sans nourriture, la partie est perdue.
- Les zombies peuvent blesser et infecter les survivants. Sans soin, une
  infection se transforme en zombie après 4 tours.
- Tous les 6 tours, si au moins 3 survivants sont en vie, un enfant rejoint le
  groupe ou un adulte change de métier. Un enfant devient adulte après 6 tours.

## Rôle des personnages

### Survivants

- **Fermier** : récolte les plantes mûres pour nourrir le groupe.
- **Docteur** : soigne les survivants infectés.
- **Militaire** : tire sur les zombies à 4 cases maximum et les poursuit,
  y compris hors de la base. Survoler un militaire met en évidence chaque case
  de sa zone de tir.
- **Constructeur** : répare les murs endommagés ou détruits. Il restaure
  3 points de durabilité par tour, ce qui lui demande plusieurs tours pour
  remettre un mur en état.
- **Enfant** : se déplace dans les environs et peut devenir adulte.

Les survivants agissent seuls. Ils attaquent un zombie voisin ou fuient le
danger avant d'effectuer leur activité habituelle.

### Zombies

- **Tank** : résistant, mais lent.
- **Runner** : rapide et se déplace de deux cases par tour.
- **Exploder** : explose lorsqu'il atteint un survivant proche.

Les zombies poursuivent les survivants, attaquent les obstacles qui bloquent
leur passage et peuvent infecter leurs victimes.

## Fin de partie

- **Défaite** : tous les survivants sont morts ou transformés, ou la réserve de
  nourriture est épuisée.
- **Victoire** : au moins un survivant est encore en vie et il n'y a plus de
  zombies.

Les vagues continuent tant que la partie n'est pas terminée.

## Interface et survol

L'interface affiche l'heure et le jour, le nombre de survivants et d'infectés,
la nourriture, la défense, le temps avant la prochaine vague et le dernier
événement du jeu.

Survoler une case de la carte affiche ses informations : par exemple, les points
de vie d'un personnage, la solidité d'un mur, la croissance d'une plante, la
quantité de nourriture d'un colis ou le temps restant avant la disparition
d'un cadavre. Survoler un militaire met en surbrillance les cases qu'il peut
atteindre avec son tir, jusqu'à 4 cases de distance.

## Contrôles

- `Espace` : lancer ou mettre en pause le déroulement automatique
- `N` ou `→` : avancer d'un tour
- `1`, `2`, `3`, `4` : régler la vitesse du déroulement automatique
- `Échap` : retourner au menu
- `F11` : activer ou quitter le plein écran
- Survoler une case : afficher ses informations

Le menu permet de choisir le nombre de survivants et de zombies, les plantes de
départ, la résistance des murs, les métiers, les types de zombies et la graine
aléatoire.

## Lancer le jeu

### Sous WSL / Ubuntu

Installer les dépendances requises : `g++`, `make`, `qmake6` et Qt 6 Widgets
(paquets Ubuntu : `qt6-base-dev` et `qt6-base-dev-tools`). Depuis la racine du
projet, compiler puis lancer :

```sh
make
make run
```

Il est aussi possible de lancer directement `./build/ZomboGIG` après la
compilation. Sous Windows avec WSL installé, `run.cmd` lance l'exécutable déjà
compilé.

## Code et utilisation de l'IA

L'IA a aidé à réaliser l'interface dans `src/ui/ui.cpp`, conformément aux
consignes. Les entités et les règles de jeu ont été écrites par
moi même.
certaines corrections ont aussi été faites avec l'aide de l'IA.
