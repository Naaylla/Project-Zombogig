#include <algorithm>
#include <vector>

class Entity;

enum class Decoration
{
    None,
    Tree,
    Bush,
    Corpse,
    Campfire
};

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

class Plant
{
public:
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

class Cell
{
public:
    Cell(int ligne, int colonne);

    int ligne() const { return position_ligne; }
    int colonne() const { return position_colonne; }

    Entity* occupant() const { return occupant_actuel; }
    void placer_occupant(Entity* entite) { occupant_actuel = entite; }
    bool est_vide() const { return occupant_actuel == nullptr; }

    bool a_un_mur() const { return mur_present; }
    Wall* mur() { return mur_present ? &mur_construit : nullptr; }
    const Wall* mur() const
    {
        return mur_present ? &mur_construit : nullptr;
    }
    void construire_mur(int durabilite,
                        Wall::Type type = Wall::Type::Mur);

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

    Decoration decoration() const { return type_decoration; }
    void changer_decoration(Decoration nouvelle)
    {
        type_decoration = nouvelle;
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
};

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
        return ligne * nombre_colonnes + colonne;
    }

    int nombre_lignes;
    int nombre_colonnes;
    std::vector<Cell> toutes_les_cellules;
};

Wall::Wall(int ligne, int colonne, int durabilite, Type type)
    : position_ligne(ligne),
      position_colonne(colonne),
      genre(type),
      ouverte(false),
      points_de_vie(durabilite),
      points_de_vie_max(durabilite)
{
}

void Wall::prendre_degats(int degats)
{
    points_de_vie = std::max(0, points_de_vie - degats);
}

void Wall::reparer(int soin)
{
    points_de_vie = std::min(points_de_vie_max,
                             points_de_vie + soin);
}

Plant::Plant(int ligne, int colonne)
    : position_ligne(ligne),
      position_colonne(colonne)
{
}

void Plant::pousser()
{
    if (niveau_croissance < croissance_max)
    {
        niveau_croissance++;
    }
}

int Plant::recolter()
{
    const int recolte = 2 + niveau_croissance / 2;

    niveau_croissance = 0;

    return recolte;
}

Cell::Cell(int ligne, int colonne)
    : position_ligne(ligne),
      position_colonne(colonne),
      mur_construit(ligne, colonne),
      plante_plantee(ligne, colonne)
{
}

void Cell::construire_mur(int durabilite, Wall::Type type)
{
    mur_construit = Wall(position_ligne, position_colonne,
                         durabilite, type);
    mur_present = true;
}

bool Cell::est_praticable() const
{
    if (mur_present && !mur_construit.est_detruit())
    {
        if (!mur_construit.est_porte())
        {
            return false;
        }

        if (!mur_construit.est_ouverte())
        {
            return false;
        }
    }

    if (type_decoration == Decoration::Tree ||
        type_decoration == Decoration::Bush ||
        type_decoration == Decoration::Campfire)
    {
        return false;
    }

    return true;
}

void Cell::reinitialiser()
{
    occupant_actuel = nullptr;
    mur_present = false;
    plante_presente = false;
    plante_plantee.reinitialiser();
    type_decoration = Decoration::None;
}

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

Cell& Grid::cellule(int ligne, int colonne)
{
    return toutes_les_cellules[indice(ligne, colonne)];
}

const Cell& Grid::cellule(int ligne, int colonne) const
{
    return toutes_les_cellules[indice(ligne, colonne)];
}

bool Grid::dans_grille(int ligne, int colonne) const
{
    return ligne >= 0 && colonne >= 0 &&
           ligne < nombre_lignes && colonne < nombre_colonnes;
}

bool Grid::est_libre(int ligne, int colonne) const
{
    if (!dans_grille(ligne, colonne))
    {
        return false;
    }

    const Cell& c = cellule(ligne, colonne);

    return c.est_praticable() && c.est_vide();
}

void Grid::reinitialiser()
{
    for (Cell& c : toutes_les_cellules)
    {
        c.reinitialiser();
    }
}
