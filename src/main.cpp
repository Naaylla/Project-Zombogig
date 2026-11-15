#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include <QApplication>
#include <QPoint>
#include <QString>
#include <QStringList>

#include "logic/world.cpp"

class Entity;
class Human;
class Zombie;

enum class Sprite : int;
enum class HumanType : int;
enum class ZombieType : int;
enum class Status : int;
enum class GameState : int;
enum class Weather : int;

class Game
{
public:
    static constexpr int intervalle_vague = 8;

    Game();

    void demarrer(int nb_survivants, int nb_zombies, int nb_plantes,
                  int durabilite_murs,
                  const std::vector<HumanType>& types_humains,
                  const std::vector<ZombieType>& types_zombies,
                  unsigned graine);
    void tour_suivant();
    void reinitialiser();

    GameState etat() const { return etat_jeu; }
    bool en_cours() const;
    bool est_termine() const;
    bool humains_gagnent() const { return victoire_humains; }
    QString texte_resultat() const;

    int tour() const { return tour_actuel; }
    int jour() const { return tour_actuel / 24 + 1; }
    int minutes_du_jour() const;
    int heure() const { return minutes_du_jour() / 60; }
    int prochaine_vague_dans() const
    {
        const int reste = tour_actuel % intervalle_vague;

        return reste == 0 ? intervalle_vague
                          : intervalle_vague - reste;
    }

    int nombre_humains() const;
    int nombre_zombies() const;
    int nombre_infectes() const;
    int nourriture() const { return stock_nourriture; }
    int defense() const;
    int nombre_murs() const;

    Weather meteo() const { return meteo_actuelle; }
    QString nom_meteo() const;
    qreal intensite_pluie() const;

    const QStringList& journal() const { return journal_evenements; }
    QString dernier_evenement() const;
    const std::vector<QPoint>& sang() const { return taches_de_sang; }

    Grid& grille() { return monde; }
    const Grid& grille() const { return monde; }

    static int distance_manhattan(int ligne1, int colonne1,
                                  int ligne2, int colonne2);
    static int distance_manhattan(const Entity& premier,
                                  const Entity& second);

    int distance_zombie_la_plus_proche(int ligne, int colonne) const;

    Human* survivant_le_plus_proche(const Entity& origine);
    Zombie* zombie_le_plus_proche(const Entity& origine, int portee_max);
    Zombie* zombie_a_cote(const Entity& origine);
    Human* infecte_le_plus_proche(const Human& origine);
    Human* infecte_a_cote(const Human& origine);
    Plant* plante_mure_a_cote(const Human& survivant);
    Plant* plante_mure_le_plus_proche(const Human& survivant,
                                      int distance_max);
    Wall* mur_abime_a_cote(const Human& survivant);
    Wall* mur_abime_le_plus_proche(const Human& survivant,
                                   int distance_max);

    void blesser_zombie(Zombie& cible, int degats);
    void exploser_zombie(Zombie& zombie);
    void ajouter_nourriture(int quantite);
    void ajouter_sang(int ligne, int colonne);
    void noter_evenement(const QString& message);
    int entier_aleatoire(int minimum, int maximum);

private:
    void construire_carte();
    void placer_survivants(int nombre);
    void placer_zombies(int nombre);
    void lancer_vague();
    bool placer_survivant(HumanType type, int ligne, int colonne);
    bool placer_zombie(ZombieType type, int ligne, int colonne);

    void avancer_meteo();
    void nourrir_survivants();
    void faire_pousser_plantes();
    void fermer_portes();
    void ouvrir_portes();
    void accueillir_survivants();
    void vieillir_enfants();

    HumanType choisir_type_humain(int index) const;
    HumanType type_humain_aleatoire();
    ZombieType choisir_type_zombie(int index) const;

    void deplacer_zombies();
    void actions_survivants();
    void infecter_exposes();
    void transformer_infectes();
    void retirer_zombies_detruits();
    void retirer_morts();
    void verifier_fin();

    Grid monde;
    std::vector<std::unique_ptr<Human>> survivants;
    std::vector<std::unique_ptr<Zombie>> horde;
    std::vector<QPoint> taches_de_sang;
    QStringList journal_evenements;
    std::mt19937 generateur;
    std::vector<HumanType> types_humains_autorises;
    std::vector<ZombieType> types_zombies_autorises;

    unsigned graine_aleatoire = 0;
    int prochain_numero = 1;
    int tour_actuel = 0;
    int stock_nourriture = 14;
    int plantes_initiales = 6;
    int durabilite_initiale_murs = 14;
    GameState etat_jeu;
    Weather meteo_actuelle;
    bool victoire_humains = false;
};

enum class GameState : int
{
    NotStarted,
    Ongoing,
    GameOver
};

enum class Weather : int
{
    Clear,
    LightRain,
    Rain,
    Clearing
};

#include "entities/human.cpp"
#include "entities/zombie.cpp"

Game::Game()
    : monde(12, 12),
      generateur(std::random_device{}())
{
    reinitialiser();
}

void Game::reinitialiser()
{
    monde.reinitialiser();
    survivants.clear();
    horde.clear();
    taches_de_sang.clear();
    journal_evenements.clear();

    graine_aleatoire = 0;
    prochain_numero = 1;
    tour_actuel = 0;
    stock_nourriture = 14;
    plantes_initiales = 6;
    durabilite_initiale_murs = 14;

    types_humains_autorises = {
        HumanType::Farmer,
        HumanType::Doctor,
        HumanType::Military,
        HumanType::Builder};

    types_zombies_autorises = {
        ZombieType::Tank,
        ZombieType::Runner,
        ZombieType::Exploder};

    etat_jeu = GameState::NotStarted;
    meteo_actuelle = Weather::LightRain;
    victoire_humains = false;
}

void Game::demarrer(int nb_survivants, int nb_zombies, int nb_plantes,
                    int durabilite_murs,
                    const std::vector<HumanType>& types_humains,
                    const std::vector<ZombieType>& types_zombies,
                    unsigned graine)
{
    reinitialiser();

    etat_jeu = GameState::Ongoing;
    graine_aleatoire = graine;
    generateur.seed(graine);

    if (!types_humains.empty())
    {
        types_humains_autorises = types_humains;
    }

    if (!types_zombies.empty())
    {
        types_zombies_autorises = types_zombies;
    }

    plantes_initiales = std::clamp(nb_plantes, 0, 6);
    durabilite_initiale_murs = std::clamp(durabilite_murs, 4, 30);

    construire_carte();
    placer_survivants(nb_survivants);
    placer_zombies(nb_zombies);

    noter_evenement(QStringLiteral(
        "Day 1, 07:00. Rain drums on the barricades."));
    noter_evenement(QStringLiteral(
        "Clear the horde. Keep the survivors fed."));
}

void Game::tour_suivant()
{
    if (!en_cours())
    {
        return;
    }

    tour_actuel++;

    fermer_portes();
    avancer_meteo();
    nourrir_survivants();
    faire_pousser_plantes();

    deplacer_zombies();
    retirer_zombies_detruits();

    infecter_exposes();
    transformer_infectes();

    ouvrir_portes();
    actions_survivants();
    retirer_zombies_detruits();
    retirer_morts();

    lancer_vague();
    accueillir_survivants();
    vieillir_enfants();
    verifier_fin();
}

void Game::nourrir_survivants()
{
    for (auto& survivant : survivants)
    {
        if (!survivant->est_vivant())
        {
            continue;
        }

        survivant->reduire_faim();

        if (survivant->faim() <= 50 && stock_nourriture > 0)
        {
            stock_nourriture--;
            survivant->manger();
        }
    }
}

void Game::faire_pousser_plantes()
{
    const bool il_pleut = intensite_pluie() > 0.0;

    for (int ligne = 0; ligne < monde.lignes(); ligne++)
    {
        for (int colonne = 0; colonne < monde.colonnes(); colonne++)
        {
            Plant* plante = monde.cellule(ligne, colonne).plante();

            if (plante == nullptr)
            {
                continue;
            }

            plante->pousser();

            if (il_pleut)
            {
                plante->pousser();
            }
        }
    }
}

void Game::fermer_portes()
{
    for (int ligne = 0; ligne < monde.lignes(); ligne++)
    {
        for (int colonne = 0; colonne < monde.colonnes(); colonne++)
        {
            Wall* mur = monde.cellule(ligne, colonne).mur();

            if (mur != nullptr && mur->est_porte() &&
                !mur->est_detruit())
            {
                mur->fermer();
            }
        }
    }
}

void Game::ouvrir_portes()
{
    const int decalage_ligne[4] = {-1, 1, 0, 0};
    const int decalage_colonne[4] = {0, 0, -1, 1};

    for (auto& survivant : survivants)
    {
        if (!survivant->est_vivant())
        {
            continue;
        }

        for (int direction = 0; direction < 4; direction++)
        {
            const int ligne =
                survivant->ligne() + decalage_ligne[direction];
            const int colonne =
                survivant->colonne() + decalage_colonne[direction];

            if (!monde.dans_grille(ligne, colonne))
            {
                continue;
            }

            Wall* mur = monde.cellule(ligne, colonne).mur();

            if (mur != nullptr && mur->est_porte() &&
                !mur->est_detruit())
            {
                mur->ouvrir();
            }
        }
    }
}

void Game::accueillir_survivants()
{
    if (tour_actuel % 6 != 0 || nombre_humains() < 3)
    {
        return;
    }

    if (entier_aleatoire(0, 99) < 55)
    {
        for (auto& survivant : survivants)
        {
            if (!survivant->est_vivant())
            {
                continue;
            }

            const int ligne = survivant->ligne();
            const int colonne = survivant->colonne();

            if (placer_survivant(HumanType::Child, ligne + 1, colonne) ||
                placer_survivant(HumanType::Child, ligne - 1, colonne) ||
                placer_survivant(HumanType::Child, ligne, colonne + 1) ||
                placer_survivant(HumanType::Child, ligne, colonne - 1))
            {
                noter_evenement(QStringLiteral(
                    "A child joins the camp."));
                return;
            }
        }

        return;
    }

    std::vector<Human*> adultes;

    for (auto& survivant : survivants)
    {
        if (survivant->est_vivant() && !survivant->est_enfant())
        {
            adultes.push_back(survivant.get());
        }
    }

    if (adultes.empty())
    {
        return;
    }

    Human* elu = adultes[static_cast<size_t>(
        entier_aleatoire(0, static_cast<int>(adultes.size()) - 1))];

    elu->changer_type(type_humain_aleatoire());
    noter_evenement(QStringLiteral("A survivor learned a new role."));
}

void Game::vieillir_enfants()
{
    for (auto& survivant : survivants)
    {
        if (!survivant->est_vivant() || !survivant->est_enfant())
        {
            continue;
        }

        survivant->vieillir();

        if (survivant->age() >= Human::age_adulte)
        {
            survivant->changer_type(type_humain_aleatoire());
            noter_evenement(QStringLiteral("A child grew up."));
        }
    }
}

void Game::deplacer_zombies()
{
    for (auto& zombie : horde)
    {
        if (zombie->est_vivant())
        {
            zombie->agir(*this);
        }
    }
}

void Game::actions_survivants()
{
    for (auto& survivant : survivants)
    {
        if (survivant->est_vivant())
        {
            survivant->agir(*this);
        }
    }
}

void Game::infecter_exposes()
{
    for (auto& survivant : survivants)
    {
        if (!survivant->est_vivant())
        {
            continue;
        }

        bool expose = false;

        for (const auto& zombie : horde)
        {
            if (zombie->est_vivant() &&
                distance_manhattan(*survivant, *zombie) <= 1)
            {
                expose = true;
                break;
            }
        }

        if (!expose)
        {
            continue;
        }

        const bool vient_d_etre_infecte =
            survivant->compteur_infection() == 0;

        survivant->infecter();

        if (vient_d_etre_infecte)
        {
            noter_evenement(survivant->nom_type() +
                            QStringLiteral(" is contaminated!"));
        }
    }
}

void Game::transformer_infectes()
{
    for (size_t index = 0; index < survivants.size();)
    {
        Human& survivant = *survivants[index];

        if (!survivant.est_vivant() || !survivant.doit_se_transformer())
        {
            index++;
            continue;
        }

        const int ligne = survivant.ligne();
        const int colonne = survivant.colonne();

        if (monde.dans_grille(ligne, colonne) &&
            monde.cellule(ligne, colonne).occupant() == &survivant)
        {
            monde.cellule(ligne, colonne).placer_occupant(nullptr);
        }

        noter_evenement(survivant.nom_type() +
                        QStringLiteral(" turned into a zombie."));

        survivants.erase(
            survivants.begin() + static_cast<long>(index));

        const ZombieType nouveau_type =
            choisir_type_zombie(entier_aleatoire(0, 99));

        auto nouveau_zombie = creer_zombie(
            prochain_numero++, nouveau_type, ligne, colonne);

        monde.cellule(ligne, colonne)
            .placer_occupant(nouveau_zombie.get());
        horde.push_back(std::move(nouveau_zombie));
    }
}

void Game::retirer_zombies_detruits()
{
    for (size_t index = 0; index < horde.size();)
    {
        Zombie& zombie = *horde[index];

        if (zombie.est_vivant())
        {
            index++;
            continue;
        }

        if (monde.dans_grille(zombie.ligne(), zombie.colonne()) &&
            monde.cellule(zombie.ligne(), zombie.colonne()).occupant() ==
                &zombie)
        {
            monde.cellule(zombie.ligne(), zombie.colonne())
                .placer_occupant(nullptr);
        }

        horde.erase(horde.begin() + static_cast<long>(index));
    }
}

void Game::retirer_morts()
{
    for (size_t index = 0; index < survivants.size();)
    {
        Human& survivant = *survivants[index];

        if (survivant.est_vivant())
        {
            index++;
            continue;
        }

        if (monde.dans_grille(survivant.ligne(), survivant.colonne()) &&
            monde.cellule(survivant.ligne(), survivant.colonne())
                    .occupant() == &survivant)
        {
            monde.cellule(survivant.ligne(), survivant.colonne())
                .placer_occupant(nullptr);
        }

        noter_evenement(QStringLiteral("A survivor died."));

        survivants.erase(
            survivants.begin() + static_cast<long>(index));
    }
}

void Game::lancer_vague()
{
    if (tour_actuel == 0 || tour_actuel % intervalle_vague != 0)
    {
        return;
    }

    const int zombies_a_placer = 3 + (tour_actuel >= 24 ? 2 : 0);

    int places = 0;

    for (int essai = 0; essai < 300 && places < zombies_a_placer; essai++)
    {
        int ligne = 0;
        int colonne = 0;

        switch (entier_aleatoire(0, 3))
        {
        case 0:
            ligne = 0;
            colonne = entier_aleatoire(1, monde.colonnes() - 2);
            break;

        case 1:
            ligne = monde.lignes() - 1;
            colonne = entier_aleatoire(1, monde.colonnes() - 2);
            break;

        case 2:
            ligne = entier_aleatoire(1, monde.lignes() - 2);
            colonne = 0;
            break;

        default:
            ligne = entier_aleatoire(1, monde.lignes() - 2);
            colonne = monde.colonnes() - 1;
            break;
        }

        const ZombieType type = choisir_type_zombie(
            entier_aleatoire(0, 99));

        if (placer_zombie(type, ligne, colonne))
        {
            places++;
        }
    }

    if (places > 0)
    {
        noter_evenement(QStringLiteral(
            "A new wave arrives from the dark."));
    }
}

void Game::verifier_fin()
{
    if (!en_cours())
    {
        return;
    }

    if (survivants.empty())
    {
        etat_jeu = GameState::GameOver;
        victoire_humains = false;
        noter_evenement(QStringLiteral("The camp has fallen."));
        return;
    }

    if (horde.empty())
    {
        etat_jeu = GameState::GameOver;
        victoire_humains = true;
        noter_evenement(QStringLiteral(
            "The last zombie falls. The camp is safe."));
        return;
    }

    if (stock_nourriture <= 0)
    {
        etat_jeu = GameState::GameOver;
        victoire_humains = false;
        noter_evenement(QStringLiteral("The camp has no food left."));
    }
}

void Game::avancer_meteo()
{
    if (tour_actuel <= 2)
    {
        meteo_actuelle = Weather::LightRain;
        return;
    }

    if (tour_actuel == 3)
    {
        meteo_actuelle = Weather::Clearing;
        noter_evenement(QStringLiteral("The rain begins to ease."));
        return;
    }

    if (tour_actuel == 4)
    {
        meteo_actuelle = Weather::Clear;
        noter_evenement(QStringLiteral(
            "The clouds break. The camp dries out."));
        return;
    }

    if (tour_actuel < 10)
    {
        meteo_actuelle = Weather::Clear;
        return;
    }

    const int tirage = entier_aleatoire(0, 99);

    switch (meteo_actuelle)
    {
    case Weather::Clear:
        if (tirage < 8)
        {
            meteo_actuelle = Weather::LightRain;
            noter_evenement(QStringLiteral(
                "Rain returns over the camp."));
        }
        break;

    case Weather::LightRain:
        if (tirage < 35)
        {
            meteo_actuelle = Weather::Rain;
            noter_evenement(QStringLiteral(
                "The rain gets heavier."));
        }
        else if (tirage < 60)
        {
            meteo_actuelle = Weather::Clearing;
        }
        break;

    case Weather::Rain:
        if (tirage < 40)
        {
            meteo_actuelle = Weather::Clearing;
            noter_evenement(QStringLiteral(
                "The storm begins to clear."));
        }
        break;

    case Weather::Clearing:
        if (tirage < 60)
        {
            meteo_actuelle = Weather::Clear;
            noter_evenement(QStringLiteral(
                "The sky brightens, still grey."));
        }
        break;
    }
}

void Game::construire_carte()
{
    for (int ligne = 3; ligne <= 9; ligne++)
    {
        for (int colonne = 3; colonne <= 9; colonne++)
        {
            const bool sur_le_bord = ligne == 3 || ligne == 9 ||
                                     colonne == 3 || colonne == 9;

            if (!sur_le_bord)
            {
                continue;
            }

            const bool sur_une_porte =
                (ligne == 3 || ligne == 9) && colonne == 6;

            if (sur_une_porte)
            {
                monde.cellule(ligne, colonne)
                    .construire_mur(10, Wall::Type::Porte);
            }
            else
            {
                monde.cellule(ligne, colonne)
                    .construire_mur(durabilite_initiale_murs);
            }
        }
    }

    monde.cellule(4, 6).construire_mur(30, Wall::Type::Maison);
    monde.cellule(8, 5).construire_mur(30, Wall::Type::Maison);

    const int positions_plantes[][2] = {
        {4, 4}, {4, 5}, {4, 7},
        {7, 4}, {7, 7}, {8, 4}};

    for (int index = 0; index < plantes_initiales; index++)
    {
        monde.cellule(positions_plantes[index][0],
                      positions_plantes[index][1])
            .planter();
    }

    monde.cellule(1, 5).changer_decoration(Decoration::Bush);
    monde.cellule(1, 9).changer_decoration(Decoration::Bush);
    monde.cellule(5, 10).changer_decoration(Decoration::Bush);
    monde.cellule(6, 10).changer_decoration(Decoration::Bush);
    monde.cellule(10, 6).changer_decoration(Decoration::Bush);

    monde.cellule(2, 2).changer_decoration(Decoration::Tree);
    monde.cellule(10, 9).changer_decoration(Decoration::Tree);
    monde.cellule(8, 7).changer_decoration(Decoration::Campfire);
    monde.cellule(2, 8).changer_decoration(Decoration::Corpse);
}

void Game::placer_survivants(int nombre)
{
    const int positions_prevues[][2] = {
        {5, 5}, {5, 6}, {6, 5}, {6, 6}, {7, 6}};

    int places = 0;

    for (const auto& position : positions_prevues)
    {
        if (places >= nombre)
        {
            break;
        }

        if (placer_survivant(choisir_type_humain(places),
                             position[0], position[1]))
        {
            places++;
        }
    }

    for (int essai = 0; places < nombre && essai < 400; essai++)
    {
        const int ligne = entier_aleatoire(4, 8);
        const int colonne = entier_aleatoire(4, 8);

        if (placer_survivant(choisir_type_humain(places),
                             ligne, colonne))
        {
            places++;
        }
    }
}

void Game::placer_zombies(int nombre)
{
    const int positions_prevues[][2] = {
        {0, 3}, {1, 10}, {3, 1}, {6, 0},
        {8, 10}, {10, 8}, {11, 2}, {11, 6}};

    int places = 0;

    for (const auto& position : positions_prevues)
    {
        if (places >= nombre)
        {
            break;
        }

        if (placer_zombie(choisir_type_zombie(places),
                          position[0], position[1]))
        {
            places++;
        }
    }

    for (int essai = 0; places < nombre && essai < 400; essai++)
    {
        const int ligne = entier_aleatoire(0, monde.lignes() - 1);
        const int colonne = entier_aleatoire(0, monde.colonnes() - 1);

        if (placer_zombie(choisir_type_zombie(places), ligne, colonne))
        {
            places++;
        }
    }
}

bool Game::placer_survivant(HumanType type, int ligne, int colonne)
{
    if (!monde.est_libre(ligne, colonne))
    {
        return false;
    }

    auto survivant = creer_humain(prochain_numero++, type,
                                  ligne, colonne);

    monde.cellule(ligne, colonne).placer_occupant(survivant.get());
    survivants.push_back(std::move(survivant));

    return true;
}

bool Game::placer_zombie(ZombieType type, int ligne, int colonne)
{
    if (!monde.est_libre(ligne, colonne))
    {
        return false;
    }

    auto zombie = creer_zombie(prochain_numero++, type,
                               ligne, colonne);

    monde.cellule(ligne, colonne).placer_occupant(zombie.get());
    horde.push_back(std::move(zombie));

    return true;
}

HumanType Game::choisir_type_humain(int index) const
{
    const int nombre_types =
        static_cast<int>(types_humains_autorises.size());

    if (nombre_types <= 0)
    {
        return HumanType::Farmer;
    }

    return types_humains_autorises[
        static_cast<size_t>(index % nombre_types)];
}

HumanType Game::type_humain_aleatoire()
{
    if (types_humains_autorises.empty())
    {
        return HumanType::Farmer;
    }

    return types_humains_autorises[static_cast<size_t>(
        entier_aleatoire(0,
                         static_cast<int>(types_humains_autorises.size()) -
                             1))];
}

ZombieType Game::choisir_type_zombie(int index) const
{
    const int nombre_types =
        static_cast<int>(types_zombies_autorises.size());

    if (nombre_types <= 0)
    {
        return ZombieType::Tank;
    }

    return types_zombies_autorises[
        static_cast<size_t>(index % nombre_types)];
}

int Game::minutes_du_jour() const
{
    return (7 * 60 + tour_actuel * 60) % (24 * 60);
}

int Game::distance_manhattan(int ligne1, int colonne1,
                             int ligne2, int colonne2)
{
    return std::abs(ligne1 - ligne2) + std::abs(colonne1 - colonne2);
}

int Game::distance_manhattan(const Entity& premier, const Entity& second)
{
    return distance_manhattan(premier.ligne(), premier.colonne(),
                              second.ligne(), second.colonne());
}

int Game::distance_zombie_la_plus_proche(int ligne, int colonne) const
{
    int distance_minimale = 999;

    for (const auto& zombie : horde)
    {
        if (!zombie->est_vivant())
        {
            continue;
        }

        distance_minimale = std::min(
            distance_minimale,
            distance_manhattan(ligne, colonne,
                               zombie->ligne(), zombie->colonne()));
    }

    return distance_minimale;
}

Human* Game::survivant_le_plus_proche(const Entity& origine)
{
    Human* meilleur = nullptr;
    int meilleure_distance = 999;

    for (auto& survivant : survivants)
    {
        if (!survivant->est_vivant())
        {
            continue;
        }

        const int distance =
            distance_manhattan(origine, *survivant);

        if (distance < meilleure_distance)
        {
            meilleure_distance = distance;
            meilleur = survivant.get();
        }
    }

    return meilleur;
}

Zombie* Game::zombie_le_plus_proche(const Entity& origine,
                                    int portee_max)
{
    Zombie* meilleur = nullptr;
    int meilleure_distance = portee_max + 1;

    for (auto& zombie : horde)
    {
        if (!zombie->est_vivant())
        {
            continue;
        }

        const int distance = distance_manhattan(origine, *zombie);

        if (distance < meilleure_distance)
        {
            meilleure_distance = distance;
            meilleur = zombie.get();
        }
    }

    return meilleur;
}

Zombie* Game::zombie_a_cote(const Entity& origine)
{
    for (auto& zombie : horde)
    {
        if (zombie->est_vivant() &&
            distance_manhattan(origine, *zombie) <= 1)
        {
            return zombie.get();
        }
    }

    return nullptr;
}

Human* Game::infecte_le_plus_proche(const Human& origine)
{
    Human* meilleur = nullptr;
    int meilleure_distance = 999;

    for (auto& survivant : survivants)
    {
        if (survivant.get() == &origine ||
            !survivant->est_vivant() ||
            survivant->compteur_infection() == 0)
        {
            continue;
        }

        const int distance = distance_manhattan(origine, *survivant);

        if (distance < meilleure_distance)
        {
            meilleure_distance = distance;
            meilleur = survivant.get();
        }
    }

    return meilleur;
}

Human* Game::infecte_a_cote(const Human& origine)
{
    for (auto& survivant : survivants)
    {
        if (survivant.get() == &origine || !survivant->est_vivant())
        {
            continue;
        }

        if (survivant->compteur_infection() > 0 &&
            distance_manhattan(origine, *survivant) <= 1)
        {
            return survivant.get();
        }
    }

    return nullptr;
}

Plant* Game::plante_mure_a_cote(const Human& survivant)
{
    for (int decalage_ligne = -1; decalage_ligne <= 1;
         decalage_ligne++)
    {
        for (int decalage_colonne = -1; decalage_colonne <= 1;
             decalage_colonne++)
        {
            const int ligne = survivant.ligne() + decalage_ligne;
            const int colonne =
                survivant.colonne() + decalage_colonne;

            if (!monde.dans_grille(ligne, colonne))
            {
                continue;
            }

            Plant* plante = monde.cellule(ligne, colonne).plante();

            if (plante != nullptr && plante->est_mure())
            {
                return plante;
            }
        }
    }

    return nullptr;
}

Plant* Game::plante_mure_le_plus_proche(const Human& survivant,
                                        int distance_max)
{
    Plant* meilleure = nullptr;
    int meilleure_distance = distance_max + 1;

    for (int ligne = 0; ligne < monde.lignes(); ligne++)
    {
        for (int colonne = 0; colonne < monde.colonnes(); colonne++)
        {
            Plant* plante = monde.cellule(ligne, colonne).plante();

            if (plante == nullptr || !plante->est_mure())
            {
                continue;
            }

            const int distance = distance_manhattan(
                survivant.ligne(), survivant.colonne(),
                ligne, colonne);

            if (distance < meilleure_distance)
            {
                meilleure_distance = distance;
                meilleure = plante;
            }
        }
    }

    return meilleure;
}

Wall* Game::mur_abime_a_cote(const Human& survivant)
{
    for (int decalage_ligne = -1; decalage_ligne <= 1;
         decalage_ligne++)
    {
        for (int decalage_colonne = -1; decalage_colonne <= 1;
             decalage_colonne++)
        {
            const int ligne = survivant.ligne() + decalage_ligne;
            const int colonne =
                survivant.colonne() + decalage_colonne;

            if (!monde.dans_grille(ligne, colonne))
            {
                continue;
            }

            Wall* mur = monde.cellule(ligne, colonne).mur();

            if (mur != nullptr &&
                !mur->est_detruit() &&
                mur->durabilite() < mur->durabilite_max())
            {
                return mur;
            }
        }
    }

    return nullptr;
}

Wall* Game::mur_abime_le_plus_proche(const Human& survivant,
                                     int distance_max)
{
    Wall* meilleur = nullptr;
    int meilleure_distance = distance_max + 1;

    for (int ligne = 0; ligne < monde.lignes(); ligne++)
    {
        for (int colonne = 0; colonne < monde.colonnes(); colonne++)
        {
            Wall* mur = monde.cellule(ligne, colonne).mur();

            if (mur == nullptr ||
                mur->est_detruit() ||
                mur->durabilite() >= mur->durabilite_max())
            {
                continue;
            }

            const int distance = distance_manhattan(
                survivant.ligne(), survivant.colonne(),
                ligne, colonne);

            if (distance < meilleure_distance)
            {
                meilleure_distance = distance;
                meilleur = mur;
            }
        }
    }

    return meilleur;
}

void Game::blesser_zombie(Zombie& cible, int degats)
{
    if (!cible.est_vivant())
    {
        return;
    }

    cible.prendre_degats(degats);

    if (cible.est_vivant())
    {
        return;
    }

    if (monde.dans_grille(cible.ligne(), cible.colonne()) &&
        monde.cellule(cible.ligne(), cible.colonne()).occupant() ==
            &cible)
    {
        monde.cellule(cible.ligne(), cible.colonne())
            .placer_occupant(nullptr);
    }

    noter_evenement(cible.nom_type() +
                    QStringLiteral(" was destroyed."));
}

void Game::exploser_zombie(Zombie& zombie)
{
    for (auto& survivant : survivants)
    {
        if (survivant->est_vivant() &&
            distance_manhattan(zombie, *survivant) <= 2)
        {
            survivant->infecter(2);
            survivant->prendre_degats(3);
            ajouter_sang(survivant->ligne(), survivant->colonne());
        }
    }

    noter_evenement(QStringLiteral(
        "An exploder bursts! Nearby survivors are hit."));

    blesser_zombie(zombie, zombie.vie_max());
}

void Game::ajouter_nourriture(int quantite)
{
    stock_nourriture = std::max(0, stock_nourriture + quantite);
}

void Game::ajouter_sang(int ligne, int colonne)
{
    const QPoint tache(colonne, ligne);

    if (std::find(taches_de_sang.begin(),
                  taches_de_sang.end(),
                  tache) == taches_de_sang.end())
    {
        taches_de_sang.push_back(tache);
    }
}

void Game::noter_evenement(const QString& message)
{
    journal_evenements.push_back(message);

    while (journal_evenements.size() > 8)
    {
        journal_evenements.removeFirst();
    }
}

QString Game::dernier_evenement() const
{
    if (journal_evenements.isEmpty())
    {
        return QStringLiteral(
            "Stay quiet. Listen for the horde.");
    }

    return journal_evenements.last();
}

int Game::entier_aleatoire(int minimum, int maximum)
{
    std::uniform_int_distribution<int> distribution(minimum, maximum);

    return distribution(generateur);
}

int Game::nombre_humains() const
{
    int vivants = 0;

    for (const auto& survivant : survivants)
    {
        if (survivant->est_vivant())
        {
            vivants++;
        }
    }

    return vivants;
}

int Game::nombre_zombies() const
{
    int vivants = 0;

    for (const auto& zombie : horde)
    {
        if (zombie->est_vivant())
        {
            vivants++;
        }
    }

    return vivants;
}

int Game::nombre_infectes() const
{
    int infectes = 0;

    for (const auto& survivant : survivants)
    {
        if (survivant->est_vivant() &&
            survivant->compteur_infection() > 0)
        {
            infectes++;
        }
    }

    return infectes;
}

int Game::defense() const
{
    int total = 0;

    for (int ligne = 0; ligne < monde.lignes(); ligne++)
    {
        for (int colonne = 0; colonne < monde.colonnes(); colonne++)
        {
            const Wall* mur = monde.cellule(ligne, colonne).mur();

            if (mur != nullptr && !mur->est_detruit())
            {
                total += mur->durabilite();
            }
        }
    }

    return total;
}

int Game::nombre_murs() const
{
    int debout = 0;

    for (int ligne = 0; ligne < monde.lignes(); ligne++)
    {
        for (int colonne = 0; colonne < monde.colonnes(); colonne++)
        {
            const Wall* mur = monde.cellule(ligne, colonne).mur();

            if (mur != nullptr && !mur->est_detruit())
            {
                debout++;
            }
        }
    }

    return debout;
}

QString Game::nom_meteo() const
{
    switch (meteo_actuelle)
    {
    case Weather::Clear:
        return QStringLiteral("CLEAR");
    case Weather::LightRain:
        return QStringLiteral("LIGHT RAIN");
    case Weather::Rain:
        return QStringLiteral("STORM");
    case Weather::Clearing:
        return QStringLiteral("CLEARING");
    }

    return QStringLiteral("CLEAR");
}

qreal Game::intensite_pluie() const
{
    switch (meteo_actuelle)
    {
    case Weather::Clear:
        return 0.0;
    case Weather::LightRain:
        return 0.45;
    case Weather::Rain:
        return 0.9;
    case Weather::Clearing:
        return 0.2;
    }

    return 0.0;
}

QString Game::texte_resultat() const
{
    if (etat_jeu == GameState::NotStarted)
    {
        return QStringLiteral("READY");
    }

    if (etat_jeu == GameState::Ongoing)
    {
        return QStringLiteral("SURVIVE");
    }

    return victoire_humains
               ? QStringLiteral("THE HORDE IS GONE")
               : QStringLiteral("THE CAMP HAS FALLEN");
}

bool Game::en_cours() const
{
    return etat_jeu == GameState::Ongoing;
}

bool Game::est_termine() const
{
    return etat_jeu == GameState::GameOver;
}

#include "ui/ui.cpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
