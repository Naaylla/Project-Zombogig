#include <algorithm>
#include <vector>

class Entity;

//liste les elements de decor qui peuvent occuper une case
enum class Decoration
{
    None,
    Tree,
    Bush,
    Corpse,
    Campfire
};

//un mur peut etre une barricade, une porte ou une maison
class Wall
{
public:
    enum class Type
    {
        Mur,
        Porte,
        Maison
    };

    Wall(int ligne, int colonne, int durabilite = 10,
         Type type = Type::Mur);

    int ligne() const { return position_ligne; }
    int colonne() const { return position_colonne; }
    Type type() const { return genre; }
    bool est_porte() const { return genre == Type::Porte; }
    bool est_maison() const { return genre == Type::Maison; }
    bool est_ouverte() const { return ouverte; }
    void ouvrir() { ouverte = true; }
    void fermer() { ouverte = false; }

    int durabilite() const { return points_de_vie; }
    int durabilite_max() const { return points_de_vie_max; }
    bool est_detruit() const { return points_de_vie <= 0; }

    void prendre_degats(int degats);
    void reparer(int soin);

private:
    int position_ligne;
    int position_colonne;
    Type genre;
    bool ouverte;
    int points_de_vie;
    int points_de_vie_max;
};

//une plante pousse puis peut etre recoltee pour nourrir le groupe
class Plant
{
public:
    //une plante est prete a recolter quand elle atteint ce niveau
    static constexpr int croissance_max = 4;

    Plant(int ligne, int colonne);

    int ligne() const { return position_ligne; }
    int colonne() const { return position_colonne; }
    int croissance() const { return niveau_croissance; }
    bool est_mure() const { return niveau_croissance >= croissance_max; }

    void pousser();
    int recolter();
    void reinitialiser() { niveau_croissance = 0; }

private:
    int position_ligne;
    int position_colonne;
    int niveau_croissance = 0;
};

//une case contient les objets et l'entite qui occupent sa position
class Cell
{
public:
    //le cadavre reste visible pendant ce nombre de tours
    static constexpr int duree_cadavre = 6;

    Cell(int ligne, int colonne);

    int ligne() const { return position_ligne; }
    int colonne() const { return position_colonne; }

    //retourne ou change l'entite placee sur la case
    Entity* occupant() const { return occupant_actuel; }
    void placer_occupant(Entity* entite) { occupant_actuel = entite; }
    bool est_vide() const { return occupant_actuel == nullptr; }

    //donne acces au mur, a la porte ou a la maison de la case
    bool a_un_mur() const { return mur_present; }
    Wall* mur() { return mur_present ? &mur_construit : nullptr; }
    const Wall* mur() const
    {
        return mur_present ? &mur_construit : nullptr;
    }
    void construire_mur(int durabilite,
                        Wall::Type type = Wall::Type::Mur);

    //donne acces a la plante cultivee sur la case
    bool a_une_plante() const { return plante_presente; }
    Plant* plante() { return plante_presente ? &plante_plantee : nullptr; }
    const Plant* plante() const
    {
        return plante_presente ? &plante_plantee : nullptr;
    }
    void planter()
    {
        plante_presente = true;
        plante_plantee.reinitialiser();
    }

    //stocke le type de decor visible sur la case
    Decoration decoration() const { return type_decoration; }
    void changer_decoration(Decoration nouvelle)
    {
        type_decoration = nouvelle;
    }

    bool a_un_cadavre() const { return tours_avant_disparition > 0; }
    int tours_cadavre() const { return tours_avant_disparition; }
    void ajouter_cadavre() { tours_avant_disparition = duree_cadavre; }
    void vieillir_cadavre()
    {
        if (tours_avant_disparition > 0)
        {
            tours_avant_disparition--;
        }
    }

    //permet a un survivant de recuperer les rations deposees ici
    int ravitaillement() const { return nourriture_deposee; }
    void deposer_ravitaillement(int quantite)
    {
        nourriture_deposee = quantite;
    }
    int ramasser_ravitaillement()
    {
        //retourne les rations puis vide la case
        const int quantite = nourriture_deposee;
        nourriture_deposee = 0;
        return quantite;
    }

    bool est_praticable() const;
    void reinitialiser();

private:
    int position_ligne;
    int position_colonne;
    Entity* occupant_actuel = nullptr;
    Wall mur_construit;
    bool mur_present = false;
    Plant plante_plantee;
    bool plante_presente = false;
    Decoration type_decoration = Decoration::None;
    int tours_avant_disparition = 0;
    int nourriture_deposee = 0;
};

//la grille rassemble toutes les cases de la carte
class Grid
{
public:
    Grid(int lignes = 12, int colonnes = 12);

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;

    int lignes() const { return nombre_lignes; }
    int colonnes() const { return nombre_colonnes; }

    Cell& cellule(int ligne, int colonne);
    const Cell& cellule(int ligne, int colonne) const;
    bool dans_grille(int ligne, int colonne) const;
    bool est_libre(int ligne, int colonne) const;

    void reinitialiser();

private:
    int indice(int ligne, int colonne) const
    {
            //range les cases ligne par ligne dans un seul vecteur
        return ligne * nombre_colonnes + colonne;
    }

    int nombre_lignes;
    int nombre_colonnes;
    std::vector<Cell> toutes_les_cellules;
};

//cree un mur avec sa durabilite et son type
Wall::Wall(int ligne, int colonne, int durabilite, Type type)
    : position_ligne(ligne),
      position_colonne(colonne),
      genre(type),
      ouverte(false),
      points_de_vie(durabilite),
      points_de_vie_max(durabilite)
{
}

//enleve des points de durabilite sans passer sous zero
void Wall::prendre_degats(int degats)
{
    points_de_vie = std::max(0, points_de_vie - degats);
}

//repare le mur sans depasser sa durabilite maximale
void Wall::reparer(int soin)
{
    points_de_vie = std::min(points_de_vie_max,
                             points_de_vie + soin);
}

//enregistre la position de la plante sur la carte
Plant::Plant(int ligne, int colonne)
    : position_ligne(ligne),
      position_colonne(colonne)
{
}

//fait pousser la plante d'un niveau jusqu'a son maximum
void Plant::pousser()
{
    if (niveau_croissance < croissance_max)
    {
        niveau_croissance++;
    }
}

int Plant::recolter()
{
    //une plante plus mature donne plus de nourriture
    const int recolte = 2 + niveau_croissance / 2;

    niveau_croissance = 0;

    return recolte;
}

//prepare une case avec une position et ses objets par defaut
Cell::Cell(int ligne, int colonne)
    : position_ligne(ligne),
      position_colonne(colonne),
      mur_construit(ligne, colonne),
      plante_plantee(ligne, colonne)
{
}

//place un nouveau mur sur cette case
void Cell::construire_mur(int durabilite, Wall::Type type)
{
    mur_construit = Wall(position_ligne, position_colonne,
                         durabilite, type);
    mur_present = true;
}

bool Cell::est_praticable() const
{
    //un mur detruit ou une porte ouverte ne bloque plus le passage
    if (mur_present && !mur_construit.est_detruit() &&
        (!mur_construit.est_porte() || !mur_construit.est_ouverte()))
    {
        return false;
    }

    //les arbres, les buissons et le feu bloquent aussi le passage
    return type_decoration != Decoration::Tree &&
           type_decoration != Decoration::Bush &&
           type_decoration != Decoration::Campfire;
}

//efface les occupants et objets pour reinitialiser la case
void Cell::reinitialiser()
{
    occupant_actuel = nullptr;
    mur_present = false;
    plante_presente = false;
    plante_plantee.reinitialiser();
    type_decoration = Decoration::None;
    tours_avant_disparition = 0;
    nourriture_deposee = 0;
}

//cree toutes les cases de la grille dans leur ordre de lecture
Grid::Grid(int lignes, int colonnes)
    : nombre_lignes(lignes),
      nombre_colonnes(colonnes)
{
    toutes_les_cellules.reserve(
        static_cast<size_t>(nombre_lignes * nombre_colonnes));

    for (int ligne = 0; ligne < nombre_lignes; ligne++)
    {
        for (int colonne = 0; colonne < nombre_colonnes; colonne++)
        {
            toutes_les_cellules.emplace_back(ligne, colonne);
        }
    }
}

//retourne la case demandee par sa ligne et sa colonne
Cell& Grid::cellule(int ligne, int colonne)
{
    return toutes_les_cellules[indice(ligne, colonne)];
}

//donne un acces en lecture seule a la case demandee
const Cell& Grid::cellule(int ligne, int colonne) const
{
    return toutes_les_cellules[indice(ligne, colonne)];
}

//verifie que les coordonnees sont comprises dans la carte
bool Grid::dans_grille(int ligne, int colonne) const
{
    return ligne >= 0 && colonne >= 0 &&
           ligne < nombre_lignes && colonne < nombre_colonnes;
}

bool Grid::est_libre(int ligne, int colonne) const
{
    //verifie les limites avant d'acceder a la case
    if (!dans_grille(ligne, colonne))
    {
        return false;
    }

    const Cell& c = cellule(ligne, colonne);

    //une case libre doit etre praticable et sans occupant
    return c.est_praticable() && c.est_vide();
}

//remet chaque case de la carte dans son etat de depart
void Grid::reinitialiser()
{
    for (Cell& c : toutes_les_cellules)
    {
        c.reinitialiser();
    }
}
