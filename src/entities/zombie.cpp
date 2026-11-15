#include <map>
#include <memory>

#include <QColor>
#include <QString>

//un zombie utilise les actions communes definies dans entity
class Zombie : public Entity
{
public:
    Zombie(int numero, ZombieType type, int ligne, int colonne);

    ZombieType type_zombie() const { return type_zombie_actuel; }
    QString nom_type() const override;
    QColor couleur() const override;
    void agir(Game& jeu) override;

private:
    int nombre_pas() const;
    void attaquer_obstacle(Game& jeu);

    ZombieType type_zombie_actuel;
};

//les differents types de zombies disponibles dans le jeu
enum class ZombieType : int
{
    Tank,
    Runner,
    Exploder
};

//regroupe les statistiques et l'apparence d'un type de zombie
struct ZombieInfo
{
    int vie_max;
    int attaque;
    int vitesse;
    QString nom;
    QColor couleur;
};

//les caracteristiques des zombies sont regroupees par type
const std::map<ZombieType, ZombieInfo> infos_zombies = {
    {ZombieType::Tank,
     {22, 3, 1, QStringLiteral("Tank Zombie"), QColor(66, 78, 62)}},
    {ZombieType::Runner,
     {8, 2, 2, QStringLiteral("Runner Zombie"), QColor(110, 116, 88)}},
    {ZombieType::Exploder,
     {11, 2, 1, QStringLiteral("Exploder Zombie"), QColor(92, 104, 74)}}};

//cherche les statistiques du type demande
const ZombieInfo& info_zombie(ZombieType type)
{
    const auto info = infos_zombies.find(type);

    if (info != infos_zombies.end())
    {
        return info->second;
    }

    //retourne un zombie standard si le type n'est pas reconnu
    static const ZombieInfo inconnu = {
        8, 2, 1, QStringLiteral("Zombie"), QColor(90, 100, 78)};
    return inconnu;
}

//cree un zombie avec les valeurs de son type
Zombie::Zombie(int numero, ZombieType type, int ligne, int colonne)
    : Entity(numero,
             info_zombie(type).vie_max,
             info_zombie(type).attaque,
             info_zombie(type).vitesse,
             ligne,
             colonne),
      type_zombie_actuel(type)
{
}

//renvoie le nom, la couleur et le sprite du type actuel
QString Zombie::nom_type() const
{
    return info_zombie(type_zombie_actuel).nom;
}

QColor Zombie::couleur() const
{
    return info_zombie(type_zombie_actuel).couleur;
}

//retourne le nombre de cases que le zombie peut parcourir par tour
int Zombie::nombre_pas() const
{
    return info_zombie(type_zombie_actuel).vitesse;
}

//cherche un mur voisin et lui inflige les degats du zombie
void Zombie::attaquer_obstacle(Game& jeu)
{
    const int decalage_ligne[4] = {-1, 1, 0, 0};
    const int decalage_colonne[4] = {0, 0, -1, 1};

    //frappe le premier mur voisin qui bloque son chemin
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

//fait agir le zombie pendant un tour
void Zombie::agir(Game& jeu)
{
    if (!est_vivant())
    {
        return;
    }

    //le zombie poursuit le survivant vivant le plus proche
    Human* cible = jeu.survivant_le_plus_proche(*this);

    if (cible == nullptr)
    {
        errer(jeu);
        return;
    }

    //l'exploder explose au lieu de poursuivre jusqu'au contact
    if (type_zombie_actuel == ZombieType::Exploder &&
        Game::distance_manhattan(*this, *cible) <= 1)
    {
        jeu.exploser_zombie(*this);
        return;
    }

    //la vitesse determine le nombre de cases parcourues ce tour
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

    //attaque la cible si le deplacement l'a amene juste a cote
    if (cible->est_vivant() &&
        Game::distance_manhattan(*this, *cible) <= 1)
    {
        cible->prendre_degats(attaque());
        jeu.ajouter_sang(cible->ligne(), cible->colonne());
        jeu.noter_evenement(nom_type() +
                            QStringLiteral(" wounds a survivor."));
    }
}

//fabrique un zombie gere par un pointeur unique
std::unique_ptr<Zombie> creer_zombie(int numero, ZombieType type, int ligne,
                                     int colonne)
{
    return std::make_unique<Zombie>(numero, type, ligne, colonne);
}
