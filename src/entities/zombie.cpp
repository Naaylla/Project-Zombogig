#include <memory>

class Zombie : public Entity
{
public:
    Zombie(int numero, ZombieType type, int ligne, int colonne);

    ZombieType type_zombie() const { return type_zombie_actuel; }
    QString nom_type() const override;
    QColor couleur() const override;
    Sprite sprite() const override;
    void agir(Game& jeu) override;

private:
    int nombre_pas() const;
    void attaquer_obstacle(Game& jeu);

    ZombieType type_zombie_actuel;
};

enum class ZombieType : int
{
    Tank,
    Runner,
    Exploder
};

struct ZombieStats
{
    int vie_max;
    int attaque;
    int vitesse;
};

ZombieStats stats_zombie(ZombieType type)
{
    switch (type)
    {
    case ZombieType::Tank:
        return {22, 3, 1};
    case ZombieType::Runner:
        return {8, 2, 2};
    case ZombieType::Exploder:
        return {11, 2, 1};
    }

    return {8, 2, 1};
}

QString nom_type_zombie(ZombieType type)
{
    switch (type)
    {
    case ZombieType::Tank:
        return QStringLiteral("Tank Zombie");
    case ZombieType::Runner:
        return QStringLiteral("Runner Zombie");
    case ZombieType::Exploder:
        return QStringLiteral("Exploder Zombie");
    }

    return QStringLiteral("Zombie");
}

QColor couleur_type_zombie(ZombieType type)
{
    switch (type)
    {
    case ZombieType::Tank:
        return QColor(66, 78, 62);
    case ZombieType::Runner:
        return QColor(110, 116, 88);
    case ZombieType::Exploder:
        return QColor(92, 104, 74);
    }

    return QColor(90, 100, 78);
}

Sprite sprite_type_zombie(ZombieType type)
{
    switch (type)
    {
    case ZombieType::Tank:
        return Sprite::Tank;
    case ZombieType::Runner:
        return Sprite::Runner;
    case ZombieType::Exploder:
        return Sprite::Exploder;
    }

    return Sprite::Tank;
}

Zombie::Zombie(int numero, ZombieType type, int ligne, int colonne)
    : Entity(numero,
             stats_zombie(type).vie_max,
             stats_zombie(type).attaque,
             stats_zombie(type).vitesse,
             ligne,
             colonne),
      type_zombie_actuel(type)
{
}

QString Zombie::nom_type() const
{
    return nom_type_zombie(type_zombie_actuel);
}

QColor Zombie::couleur() const
{
    return couleur_type_zombie(type_zombie_actuel);
}

Sprite Zombie::sprite() const
{
    return sprite_type_zombie(type_zombie_actuel);
}

int Zombie::nombre_pas() const
{
    return type_zombie_actuel == ZombieType::Runner ? 2 : 1;
}

void Zombie::attaquer_obstacle(Game& jeu)
{
    const int decalage_ligne[4] = {-1, 1, 0, 0};
    const int decalage_colonne[4] = {0, 0, -1, 1};

    for (int direction = 0; direction < 4; direction++)
    {
        const int ligne_voisine = ligne() + decalage_ligne[direction];
        const int colonne_voisine =
            colonne() + decalage_colonne[direction];

        if (!jeu.grille().dans_grille(ligne_voisine, colonne_voisine))
        {
            continue;
        }

        Cell& cellule = jeu.grille().cellule(ligne_voisine,
                                             colonne_voisine);
        Wall* obstacle = cellule.mur();

        if (obstacle == nullptr || obstacle->est_detruit())
        {
            continue;
        }

        obstacle->prendre_degats(attaque());
        jeu.noter_evenement(nom_type() +
                            QStringLiteral(" breaks a barricade."));
        return;
    }
}

void Zombie::agir(Game& jeu)
{
    if (!est_vivant())
    {
        return;
    }

    Human* cible = jeu.survivant_le_plus_proche(*this);

    if (cible == nullptr)
    {
        errer(jeu);
        return;
    }

    if (type_zombie_actuel == ZombieType::Exploder &&
        Game::distance_manhattan(*this, *cible) <= 1)
    {
        jeu.exploser_zombie(*this);
        return;
    }

    for (int pas = 0; pas < nombre_pas(); pas++)
    {
        if (!cible->est_vivant() ||
            Game::distance_manhattan(*this, *cible) <= 1)
        {
            break;
        }

        if (avancer_vers(jeu, cible->ligne(), cible->colonne()))
        {
            continue;
        }

        attaquer_obstacle(jeu);
        break;
    }

    if (cible->est_vivant() &&
        Game::distance_manhattan(*this, *cible) <= 1)
    {
        cible->prendre_degats(attaque());
        jeu.ajouter_sang(cible->ligne(), cible->colonne());
        jeu.noter_evenement(nom_type() +
                            QStringLiteral(" wounds a survivor."));
    }
}

std::unique_ptr<Zombie> creer_zombie(int numero, ZombieType type, int ligne,
                                     int colonne)
{
    return std::make_unique<Zombie>(numero, type, ligne, colonne);
}
