#include <algorithm>
#include <memory>

class Game;

enum class Sprite : int
{
    Farmer,
    Doctor,
    Military,
    Builder,
    Child,
    Tank,
    Runner,
    Exploder
};

enum class HumanType : int
{
    Farmer,
    Doctor,
    Military,
    Builder,
    Child
};

enum class Status : int
{
    Healthy,
    Infected,
    Transformed
};

class Entity
{
public:
    Entity(int numero, int vie_max, int attaque, int vitesse,
           int ligne, int colonne);
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    int id() const { return numero; }
    int vie() const { return points_de_vie; }
    int vie_max() const { return points_de_vie_max; }
    int attaque() const { return force; }
    int vitesse() const { return pas; }
    bool est_vivant() const { return points_de_vie > 0; }

    int ligne() const { return position_ligne; }
    int colonne() const { return position_colonne; }
    void changer_position(int ligne, int colonne);

    virtual QString nom_type() const = 0;
    virtual QColor couleur() const = 0;
    virtual Sprite sprite() const = 0;
    virtual void agir(Game& jeu) = 0;

    void prendre_degats(int degats);
    void soigner(int soin);
    void changer_stats(int vie_max, int attaque, int vitesse);

    bool essayer_deplacement(Game& jeu, int delta_ligne,
                             int delta_colonne);
    bool avancer_vers(Game& jeu, int ligne_cible, int colonne_cible);
    bool errer(Game& jeu);

private:
    int numero;
    int points_de_vie;
    int points_de_vie_max;
    int force;
    int pas;
    int position_ligne;
    int position_colonne;
};

class Human : public Entity
{
public:
    Human(int numero, HumanType type, int ligne, int colonne);

    HumanType type_humain() const { return type_humain_actuel; }
    void changer_type(HumanType type);
    QString nom_type() const override;
    QColor couleur() const override;
    Sprite sprite() const override;
    void agir(Game& jeu) override;

    Status etat() const { return etat_sante; }
    int compteur_infection() const { return infection; }
    int faim() const { return niveau_faim; }
    int age() const { return nombre_tours_vecu; }
    bool est_enfant() const { return type_humain_actuel == HumanType::Child; }
    bool doit_se_transformer() const
    {
        return infection >= duree_infection;
    }

    void infecter(int quantite = 1);
    void guerir();
    void reduire_faim();
    void manger();
    void vieillir();

    bool fuir_le_danger(Game& jeu);

    static constexpr int duree_infection = 4;
    static constexpr int age_adulte = 6;

private:
    HumanType type_humain_actuel;
    Status etat_sante;
    int infection = 0;
    int niveau_faim = 100;
    int nombre_tours_vecu = 0;
    int recharge = 0;
};

struct HumanStats
{
    int vie_max;
    int attaque;
    int vitesse;
};

HumanStats stats_humain(HumanType type)
{
    switch (type)
    {
    case HumanType::Farmer:
        return {14, 4, 1};
    case HumanType::Doctor:
        return {12, 3, 1};
    case HumanType::Military:
        return {20, 11, 1};
    case HumanType::Builder:
        return {16, 5, 1};
    case HumanType::Child:
        return {6, 1, 1};
    }

    return {10, 3, 1};
}

QString nom_type_humain(HumanType type)
{
    switch (type)
    {
    case HumanType::Farmer:
        return QStringLiteral("Farmer");
    case HumanType::Doctor:
        return QStringLiteral("Doctor");
    case HumanType::Military:
        return QStringLiteral("Military");
    case HumanType::Builder:
        return QStringLiteral("Builder");
    case HumanType::Child:
        return QStringLiteral("Child");
    }

    return QStringLiteral("Survivor");
}

QColor couleur_type_humain(HumanType type)
{
    switch (type)
    {
    case HumanType::Farmer:
        return QColor(122, 104, 80);
    case HumanType::Doctor:
        return QColor(150, 148, 140);
    case HumanType::Military:
        return QColor(84, 92, 66);
    case HumanType::Builder:
        return QColor(130, 92, 56);
    case HumanType::Child:
        return QColor(170, 140, 104);
    }

    return QColor(120, 110, 90);
}

Sprite sprite_type_humain(HumanType type)
{
    switch (type)
    {
    case HumanType::Farmer:
        return Sprite::Farmer;
    case HumanType::Doctor:
        return Sprite::Doctor;
    case HumanType::Military:
        return Sprite::Military;
    case HumanType::Builder:
        return Sprite::Builder;
    case HumanType::Child:
        return Sprite::Child;
    }

    return Sprite::Farmer;
}

namespace
{

int signe(int valeur)
{
    return (valeur > 0) - (valeur < 0);
}

}

Entity::Entity(int numero, int vie_max, int attaque, int vitesse,
               int ligne, int colonne)
    : numero(numero),
      points_de_vie(vie_max),
      points_de_vie_max(vie_max),
      force(attaque),
      pas(vitesse),
      position_ligne(ligne),
      position_colonne(colonne)
{
}

void Entity::changer_position(int ligne, int colonne)
{
    position_ligne = ligne;
    position_colonne = colonne;
}

void Entity::prendre_degats(int degats)
{
    points_de_vie = std::max(0, points_de_vie - degats);
}

void Entity::soigner(int soin)
{
    points_de_vie = std::min(points_de_vie_max, points_de_vie + soin);
}

void Entity::changer_stats(int vie_max, int attaque, int vitesse)
{
    points_de_vie_max = vie_max;
    points_de_vie = vie_max;
    force = attaque;
    pas = vitesse;
}

bool Entity::essayer_deplacement(Game& jeu, int delta_ligne,
                                 int delta_colonne)
{
    if (delta_ligne == 0 && delta_colonne == 0)
    {
        return false;
    }

    const int nouvelle_ligne = position_ligne + delta_ligne;
    const int nouvelle_colonne = position_colonne + delta_colonne;

    Grid& grille = jeu.grille();

    if (!grille.est_libre(nouvelle_ligne, nouvelle_colonne))
    {
        return false;
    }

    Cell& depart = grille.cellule(position_ligne, position_colonne);

    if (depart.occupant() == this)
    {
        depart.placer_occupant(nullptr);
    }

    grille.cellule(nouvelle_ligne, nouvelle_colonne)
        .placer_occupant(this);

    changer_position(nouvelle_ligne, nouvelle_colonne);

    return true;
}

bool Entity::avancer_vers(Game& jeu, int ligne_cible,
                          int colonne_cible)
{
    const int delta_ligne = signe(ligne_cible - position_ligne);
    const int delta_colonne = signe(colonne_cible - position_colonne);

    if (delta_ligne != 0 &&
        essayer_deplacement(jeu, delta_ligne, 0))
    {
        return true;
    }

    if (delta_colonne != 0 &&
        essayer_deplacement(jeu, 0, delta_colonne))
    {
        return true;
    }

    return false;
}

bool Entity::errer(Game& jeu)
{
    const int decalage_ligne[4] = {-1, 1, 0, 0};
    const int decalage_colonne[4] = {0, 0, -1, 1};

    int directions_possibles[4];
    int nombre_possibilites = 0;

    for (int direction = 0; direction < 4; direction++)
    {
        if (jeu.grille().est_libre(
                position_ligne + decalage_ligne[direction],
                position_colonne + decalage_colonne[direction]))
        {
            directions_possibles[nombre_possibilites] = direction;
            nombre_possibilites++;
        }
    }

    if (nombre_possibilites == 0)
    {
        return false;
    }

    const int choix =
        jeu.entier_aleatoire(0, nombre_possibilites - 1);
    const int direction = directions_possibles[choix];

    return essayer_deplacement(jeu, decalage_ligne[direction],
                               decalage_colonne[direction]);
}

Human::Human(int numero, HumanType type, int ligne, int colonne)
    : Entity(numero,
             stats_humain(type).vie_max,
             stats_humain(type).attaque,
             stats_humain(type).vitesse,
             ligne,
             colonne),
      type_humain_actuel(type),
      etat_sante(Status::Healthy)
{
}

void Human::changer_type(HumanType type)
{
    type_humain_actuel = type;
    changer_stats(stats_humain(type).vie_max,
                  stats_humain(type).attaque,
                  stats_humain(type).vitesse);
}

QString Human::nom_type() const
{
    return nom_type_humain(type_humain_actuel);
}

QColor Human::couleur() const
{
    return couleur_type_humain(type_humain_actuel);
}

Sprite Human::sprite() const
{
    return sprite_type_humain(type_humain_actuel);
}

void Human::agir(Game& jeu)
{
    if (!est_vivant())
    {
        return;
    }

    if (Zombie* menace = jeu.zombie_a_cote(*this))
    {
        jeu.blesser_zombie(*menace, attaque());
        fuir_le_danger(jeu);
        return;
    }

    switch (type_humain_actuel)
    {
    case HumanType::Child:
        if (jeu.entier_aleatoire(0, 99) < 40)
        {
            errer(jeu);
        }
        return;

    case HumanType::Farmer:
    {
        Plant* plante = jeu.plante_mure_a_cote(*this);

        if (plante == nullptr)
        {
            plante = jeu.plante_mure_le_plus_proche(*this, 8);
        }

        if (plante == nullptr)
        {
            errer(jeu);
            return;
        }

        if (jeu.plante_mure_a_cote(*this) != nullptr)
        {
            jeu.ajouter_nourriture(plante->recolter());
            jeu.noter_evenement(
                QStringLiteral("A farmer harvested food."));
            return;
        }

        if (!avancer_vers(jeu, plante->ligne(), plante->colonne()))
        {
            errer(jeu);
        }

        return;
    }

    case HumanType::Doctor:
    {
        Human* patient = jeu.infecte_a_cote(*this);

        if (patient != nullptr)
        {
            patient->guerir();
            jeu.noter_evenement(
                QStringLiteral("A doctor cured a survivor."));
            return;
        }

        patient = jeu.infecte_le_plus_proche(*this);

        if (patient != nullptr &&
            !avancer_vers(jeu, patient->ligne(), patient->colonne()))
        {
            errer(jeu);
            return;
        }

        if (patient == nullptr)
        {
            errer(jeu);
        }

        return;
    }

    case HumanType::Military:
    {
        if (recharge > 0)
        {
            recharge--;
        }

        Zombie* cible = jeu.zombie_le_plus_proche(*this, 4);

        if (cible == nullptr)
        {
            errer(jeu);
            return;
        }

        if (recharge == 0)
        {
            jeu.blesser_zombie(*cible, attaque());
            recharge = 1;
            jeu.noter_evenement(
                QStringLiteral("The military shot a zombie."));
        }

        return;
    }

    case HumanType::Builder:
    {
        Wall* mur = jeu.mur_abime_a_cote(*this);

        if (mur != nullptr)
        {
            mur->reparer(4);
            jeu.noter_evenement(
                QStringLiteral("A builder repaired a wall."));
            return;
        }

        mur = jeu.mur_abime_le_plus_proche(*this, 8);

        if (mur == nullptr)
        {
            errer(jeu);
            return;
        }

        if (!avancer_vers(jeu, mur->ligne(), mur->colonne()))
        {
            errer(jeu);
        }

        return;
    }
    }
}

void Human::infecter(int quantite)
{
    if (etat_sante == Status::Transformed)
    {
        return;
    }

    infection = std::min(duree_infection, infection + quantite);

    if (infection > 0)
    {
        etat_sante = Status::Infected;
    }
}

void Human::guerir()
{
    infection = 0;
    etat_sante = Status::Healthy;
    soigner(2);
}

void Human::reduire_faim()
{
    niveau_faim = std::max(0, niveau_faim - 15);

    if (niveau_faim == 0)
    {
        prendre_degats(2);
    }
}

void Human::manger()
{
    niveau_faim = std::min(100, niveau_faim + 45);
}

void Human::vieillir()
{
    if (type_humain_actuel != HumanType::Child)
    {
        return;
    }

    nombre_tours_vecu++;
}

bool Human::fuir_le_danger(Game& jeu)
{
    const int decalage_ligne[4] = {-1, 1, 0, 0};
    const int decalage_colonne[4] = {0, 0, -1, 1};

    int meilleure_ligne = -1;
    int meilleure_colonne = -1;
    int meilleure_distance = -1;

    for (int direction = 0; direction < 4; direction++)
    {
        const int ligne = this->ligne() + decalage_ligne[direction];
        const int colonne =
            this->colonne() + decalage_colonne[direction];

        if (!jeu.grille().est_libre(ligne, colonne))
        {
            continue;
        }

        const int distance =
            jeu.distance_zombie_la_plus_proche(ligne, colonne);

        if (distance > meilleure_distance)
        {
            meilleure_distance = distance;
            meilleure_ligne = ligne;
            meilleure_colonne = colonne;
        }
    }

    if (meilleure_ligne < 0)
    {
        return false;
    }

    return essayer_deplacement(jeu,
                               meilleure_ligne - ligne(),
                               meilleure_colonne - colonne());
}

std::unique_ptr<Human> creer_humain(int numero, HumanType type, int ligne,
                                    int colonne)
{
    return std::make_unique<Human>(numero, type, ligne, colonne);
}
