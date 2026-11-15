#include <algorithm>
#include <map>
#include <memory>

#include <QColor>
#include <QString>

class Game;

//les metiers donnent des actions et des statistiques differentes
enum class HumanType : int
{
    Farmer,
    Doctor,
    Military,
    Builder,
    Child
};

//indique si un humain est sain, infecte ou transforme
enum class Status : int
{
    Healthy,
    Infected,
    Transformed
};

//classe de base commune aux survivants et aux zombies
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

//un humain garde son metier, sa faim, son age et son infection
class Human : public Entity
{
public:
    Human(int numero, HumanType type, int ligne, int colonne);

    HumanType type_humain() const { return type_humain_actuel; }
    void changer_type(HumanType type);
    QString nom_type() const override;
    QColor couleur() const override;
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
    static constexpr int portee_tir = 4;

private:
    HumanType type_humain_actuel;
    Status etat_sante;
    int infection = 0;
    int niveau_faim = 100;
    int nombre_tours_vecu = 0;
    int recharge = 0;
};

//regroupe les statistiques et l'apparence d'un metier
struct HumanInfo
{
    int vie_max;
    int attaque;
    int vitesse;
    QString nom;
    QColor couleur;
};

//les caracteristiques de chaque metier sont regroupees ici
const std::map<HumanType, HumanInfo> infos_humains = {
    {HumanType::Farmer,
     {14, 4, 1, QStringLiteral("Farmer"), QColor(122, 104, 80)}},
    {HumanType::Doctor,
     {12, 3, 1, QStringLiteral("Doctor"), QColor(150, 148, 140)}},
    {HumanType::Military,
     {20, 11, 1, QStringLiteral("Military"), QColor(84, 92, 66)}},
    {HumanType::Builder,
     {16, 5, 1, QStringLiteral("Builder"), QColor(130, 92, 56)}},
    {HumanType::Child,
     {6, 1, 1, QStringLiteral("Child"), QColor(170, 140, 104)}}};

//cherche les statistiques du metier demande
const HumanInfo& info_humain(HumanType type)
{
    const auto info = infos_humains.find(type);

    if (info != infos_humains.end())
    {
        return info->second;
    }

    //utilise des valeurs simples si le metier n'est pas reconnu
    static const HumanInfo inconnu = {
        10, 3, 1, QStringLiteral("Survivor"), QColor(120, 110, 90)};
    return inconnu;
}

namespace
{

//renvoie -1, 0 ou 1 selon le sens du deplacement
int signe(int valeur)
{
    return (valeur > 0) - (valeur < 0);
}

}

//initialise les points de vie et la position de l'entite
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

//met a jour la position en memoire
void Entity::changer_position(int ligne, int colonne)
{
    position_ligne = ligne;
    position_colonne = colonne;
}

//retire des points de vie sans passer sous zero
void Entity::prendre_degats(int degats)
{
    points_de_vie = std::max(0, points_de_vie - degats);
}

//rend des points de vie sans depasser le maximum
void Entity::soigner(int soin)
{
    points_de_vie = std::min(points_de_vie_max, points_de_vie + soin);
}

//remplace les statistiques quand un humain change de metier
void Entity::changer_stats(int vie_max, int attaque, int vitesse)
{
    points_de_vie_max = vie_max;
    points_de_vie = vie_max;
    force = attaque;
    pas = vitesse;
}

//deplace l'entite seulement si la case d'arrivee est libre
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

//essaie de rejoindre la cible en se deplacant sur une seule case
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

//choisit au hasard une case libre autour de l'entite
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

//cree un survivant avec les statistiques de son metier
Human::Human(int numero, HumanType type, int ligne, int colonne)
    : Entity(numero,
             info_humain(type).vie_max,
             info_humain(type).attaque,
             info_humain(type).vitesse,
             ligne,
             colonne),
      type_humain_actuel(type),
      etat_sante(Status::Healthy)
{
}

//change le metier et remet ses statistiques a leur valeur de base
void Human::changer_type(HumanType type)
{
    type_humain_actuel = type;
    changer_stats(info_humain(type).vie_max,
                  info_humain(type).attaque,
                  info_humain(type).vitesse);
}

//fournit le nom, la couleur et le sprite du metier actuel
QString Human::nom_type() const
{
    return info_humain(type_humain_actuel).nom;
}

QColor Human::couleur() const
{
    return info_humain(type_humain_actuel).couleur;
}

//choisit l'action du survivant pour ce tour
void Human::agir(Game& jeu)
{
    if (!est_vivant())
    {
        return;
    }

    //un zombie voisin passe avant le travail du metier
    if (Zombie* menace = jeu.zombie_a_cote(*this))
    {
        jeu.blesser_zombie(*menace, attaque());
        fuir_le_danger(jeu);
        return;
    }

    //chaque metier cherche ensuite une cible liee a son role
    switch (type_humain_actuel)
    {
    case HumanType::Child:
        //l'enfant se deplace parfois sans objectif precis
        if (jeu.entier_aleatoire(0, 99) < 40)
        {
            errer(jeu);
        }
        return;

    case HumanType::Farmer:
    {
        //le fermier cherche une plante mure, d'abord a cote puis plus loin
        Plant* plante = jeu.plante_mure_a_cote(*this);
        const bool plante_voisine = plante != nullptr;

        if (plante == nullptr)
        {
            plante = jeu.plante_mure_le_plus_proche(*this, 8);
        }

        if (plante == nullptr)
        {
            errer(jeu);
            return;
        }

        if (plante_voisine)
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
        //le docteur soigne un infecte voisin ou se rapproche du plus proche
        Human* patient = jeu.infecte_a_cote(*this);

        if (patient != nullptr)
        {
            patient->guerir();
            jeu.noter_evenement(
                QStringLiteral("A doctor cured a survivor."));
            return;
        }

        patient = jeu.infecte_le_plus_proche(*this);

        if (patient == nullptr ||
            !avancer_vers(jeu, patient->ligne(), patient->colonne()))
        {
            errer(jeu);
        }

        return;
    }

    case HumanType::Military:
    {
        //le militaire tire sur le zombie le plus proche dans sa portee
        if (recharge > 0)
        {
            recharge--;
        }

        Zombie* cible = jeu.zombie_le_plus_proche(*this, portee_tir);

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
        //le constructeur repare un mur abime avant de chercher plus loin
        Wall* mur = jeu.mur_abime_a_cote(*this);

        if (mur != nullptr)
        {
            mur->reparer(3);
            jeu.noter_evenement(
                QStringLiteral("A builder repaired a wall."));
            return;
        }

        mur = jeu.mur_abime_le_plus_proche(*this, 8);

        if (mur == nullptr ||
            !avancer_vers(jeu, mur->ligne(), mur->colonne()))
        {
            errer(jeu);
        }

        return;
    }
    }
}

//ajoute de l'infection et change l'etat de sante
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

//soigne l'infection et rend un peu de vie
void Human::guerir()
{
    infection = 0;
    etat_sante = Status::Healthy;
    soigner(2);
}

void Human::reduire_faim()
{
    //la faim baisse a chaque tour et cause des degats quand elle atteint zero
    niveau_faim = std::max(0, niveau_faim - 15);

    if (niveau_faim == 0)
    {
        prendre_degats(2);
    }
}

void Human::manger()
{
    //la faim ne peut pas depasser son maximum
    niveau_faim = std::min(100, niveau_faim + 45);
}

//fait avancer l'age d'un enfant d'un tour
void Human::vieillir()
{
    if (type_humain_actuel != HumanType::Child)
    {
        return;
    }

    nombre_tours_vecu++;
}

//cherche une case voisine qui augmente la distance avec les zombies
bool Human::fuir_le_danger(Game& jeu)
{
    const int decalage_ligne[4] = {-1, 1, 0, 0};
    const int decalage_colonne[4] = {0, 0, -1, 1};

    int meilleure_ligne = -1;
    int meilleure_colonne = -1;
    int meilleure_distance = -1;

    //choisit la case libre la plus eloignee des zombies
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

//fabrique un humain gere par un pointeur unique
std::unique_ptr<Human> creer_humain(int numero, HumanType type, int ligne,
                                    int colonne)
{
    return std::make_unique<Human>(numero, type, ligne, colonne);
}
