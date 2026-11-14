#include "logic/game.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "logic/plant.h"
#include "logic/wall.h"

namespace {

int sign(int value) {
    return (value > 0) - (value < 0);
}

int manhattanStatic(int rowA, int colA, int rowB, int colB) {
    return std::abs(rowA - rowB) + std::abs(colA - colB);
}

}  // namespace

Game::Game()
    : grid_(12, 12),
      rng_(std::random_device{}()) {
    reset();
}

void Game::reset() {
    grid_.reset();
    humans_.clear();
    zombies_.clear();
    blood_.clear();
    log_.clear();

    seed_ = 0;
    nextId_ = 1;
    turn_ = 0;
    food_ = 14;
    state_ = GameState::NotStarted;
    weather_ = Weather::LightRain;
    humansWon_ = false;
}

void Game::start(int survivors, int zombies, unsigned seed) {
    reset();

    state_ = GameState::Ongoing;
    seed_ = seed;
    rng_.seed(seed);

    buildMap();
    spawnInitialHumans(survivors);
    spawnInitialZombies(zombies);

    log(QStringLiteral("Day 1, 07:00. Rain drums on the barricades."));
    log(QStringLiteral("Survive 48 hours. Keep the camp fed."));
}

void Game::buildMap() {
    const int walls[][2] = {
        {4, 4}, {4, 5}, {4, 6}, {4, 7},
        {5, 4}, {5, 7},
        {6, 4}, {6, 7},
        {7, 4}, {7, 5}, {7, 7},
        {8, 4}, {8, 5}};
    for (const auto& wall : walls) {
        grid_.cell(wall[0], wall[1]).buildWall(14);
    }

    const int plants[][2] = {
        {9, 3}, {9, 4}, {10, 3}, {10, 4}, {8, 3}, {11, 4}};
    for (const auto& plant : plants) {
        grid_.cell(plant[0], plant[1]).seedPlant();
    }

    grid_.cell(1, 5).setDecoration(Decoration::Bush);
    grid_.cell(1, 9).setDecoration(Decoration::Bush);
    grid_.cell(5, 10).setDecoration(Decoration::Bush);
    grid_.cell(6, 10).setDecoration(Decoration::Bush);
    grid_.cell(10, 6).setDecoration(Decoration::Bush);

    grid_.cell(2, 2).setDecoration(Decoration::Tree);
    grid_.cell(9, 9).setDecoration(Decoration::Tree);
    grid_.cell(8, 7).setDecoration(Decoration::Campfire);
    grid_.cell(3, 8).setDecoration(Decoration::Corpse);
}

void Game::spawnInitialHumans(int count) {
    struct Spot {
        int row;
        int col;
        HumanType type;
    };
    const Spot spots[] = {
        {5, 5, HumanType::Farmer},
        {5, 6, HumanType::Builder},
        {6, 5, HumanType::Doctor},
        {6, 6, HumanType::Military},
        {9, 6, HumanType::Military}};

    int placed = 0;
    for (const Spot& spot : spots) {
        if (placed >= count) {
            break;
        }
        if (spawnHuman(spot.type, spot.row, spot.col)) {
            ++placed;
        }
    }

    for (int attempt = 0; placed < count && attempt < 400; ++attempt) {
        const int row = randomInt(1, grid_.rows() - 2);
        const int col = randomInt(1, grid_.cols() - 2);
        const auto type = static_cast<HumanType>(placed % 4);
        if (spawnHuman(type, row, col)) {
            ++placed;
        }
    }
}

void Game::spawnInitialZombies(int count) {
    struct Spot {
        int row;
        int col;
        ZombieType type;
    };
    const Spot spots[] = {
        {0, 3, ZombieType::Tank},
        {1, 10, ZombieType::Runner},
        {3, 1, ZombieType::Runner},
        {6, 0, ZombieType::Tank},
        {8, 10, ZombieType::Exploder},
        {10, 8, ZombieType::Exploder},
        {11, 2, ZombieType::Tank},
        {11, 6, ZombieType::Runner}};

    int placed = 0;
    for (const Spot& spot : spots) {
        if (placed >= count) {
            break;
        }
        if (spawnZombie(spot.type, spot.row, spot.col)) {
            ++placed;
        }
    }

    for (int attempt = 0; placed < count && attempt < 400; ++attempt) {
        const int row = randomInt(1, grid_.rows() - 2);
        const int col = randomInt(1, grid_.cols() - 2);
        const auto type = static_cast<ZombieType>(placed % 3);
        if (spawnZombie(type, row, col)) {
            ++placed;
        }
    }
}

bool Game::spawnHuman(HumanType type, int row, int col) {
    if (!grid_.isFree(row, col)) {
        return false;
    }
    auto human = makeHuman(nextId_++, type, row, col);
    grid_.cell(row, col).setOccupant(human.get());
    humans_.push_back(std::move(human));
    return true;
}

bool Game::spawnZombie(ZombieType type, int row, int col) {
    if (!grid_.isFree(row, col)) {
        return false;
    }
    auto zombie = makeZombie(nextId_++, type, row, col);
    grid_.cell(row, col).setOccupant(zombie.get());
    zombies_.push_back(std::move(zombie));
    return true;
}

void Game::nextTurn() {
    if (!isRunning()) {
        return;
    }

    ++turn_;
    updateWeather();
    updateFood();
    growPlants();

    moveZombies();
    removeDeadZombies();

    infectHumans();
    transformHumans();

    actHumans();
    removeDeadZombies();
    removeDeadHumans();

    spawnWave();
    checkEndCondition();
}

void Game::updateWeather() {
    if (turn_ <= 2) {
        weather_ = Weather::LightRain;
        return;
    }
    if (turn_ == 3) {
        weather_ = Weather::Clearing;
        log(QStringLiteral("The rain begins to ease."));
        return;
    }
    if (turn_ == 4) {
        weather_ = Weather::Clear;
        log(QStringLiteral("The clouds break. The camp dries out."));
        return;
    }
    if (turn_ < 10) {
        weather_ = Weather::Clear;
        return;
    }

    const int roll = randomInt(0, 99);
    switch (weather_) {
    case Weather::Clear:
        if (roll < 8) {
            weather_ = Weather::LightRain;
            log(QStringLiteral("Rain returns over the camp."));
        }
        break;
    case Weather::LightRain:
        if (roll < 35) {
            weather_ = Weather::Rain;
            log(QStringLiteral("The rain gets heavier."));
        } else if (roll < 60) {
            weather_ = Weather::Clearing;
        }
        break;
    case Weather::Rain:
        if (roll < 40) {
            weather_ = Weather::Clearing;
            log(QStringLiteral("The storm begins to clear."));
        }
        break;
    case Weather::Clearing:
        if (roll < 60) {
            weather_ = Weather::Clear;
            log(QStringLiteral("The sky brightens, still grey."));
        }
        break;
    }
}

void Game::updateFood() {
    for (auto& human : humans_) {
        if (!human->alive()) {
            continue;
        }
        human->loseHunger();
        if (human->wantsFood() && food_ > 0) {
            --food_;
            human->eat();
        }
    }
}

void Game::growPlants() {
    const bool raining = rainIntensity() > 0.0;
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            Plant* plant = grid_.cell(row, col).plant();
            if (!plant) {
                continue;
            }
            plant->grow();
            if (raining) {
                plant->grow();
            }
        }
    }
}

void Game::moveZombies() {
    for (auto& zombie : zombies_) {
        if (zombie->alive()) {
            zombie->update(*this);
        }
    }
}

void Game::actHumans() {
    for (auto& human : humans_) {
        if (human->alive()) {
            human->update(*this);
        }
    }
}

void Game::infectHumans() {
    for (auto& human : humans_) {
        if (!human->alive()) {
            continue;
        }
        bool exposed = false;
        for (const auto& zombie : zombies_) {
            if (zombie->alive() && manhattan(*human, *zombie) <= 1) {
                exposed = true;
                break;
            }
        }
        if (!exposed) {
            continue;
        }
        const bool newlyInfected = human->contaminationTimer() == 0;
        human->getInfected();
        if (newlyInfected) {
            log(human->typeName() + QStringLiteral(" is contaminated!"));
        }
    }
}

void Game::transformHumans() {
    for (size_t i = 0; i < humans_.size();) {
        Human& human = *humans_[i];
        if (!human.alive() || !human.shouldTransform()) {
            ++i;
            continue;
        }

        const int row = human.row();
        const int col = human.col();
        if (grid_.isInside(row, col) &&
            grid_.cell(row, col).occupant() == &human) {
            grid_.cell(row, col).setOccupant(nullptr);
        }

        log(human.typeName() + QStringLiteral(" turned into a zombie."));
        humans_.erase(humans_.begin() + static_cast<long>(i));

        const auto type = static_cast<ZombieType>(randomInt(0, 2));
        auto zombie = makeZombie(nextId_++, type, row, col);
        grid_.cell(row, col).setOccupant(zombie.get());
        zombies_.push_back(std::move(zombie));
    }
}

void Game::removeDeadZombies() {
    for (size_t i = 0; i < zombies_.size();) {
        Zombie& zombie = *zombies_[i];
        if (zombie.alive()) {
            ++i;
            continue;
        }
        if (grid_.isInside(zombie.row(), zombie.col()) &&
            grid_.cell(zombie.row(), zombie.col()).occupant() == &zombie) {
            grid_.cell(zombie.row(), zombie.col()).setOccupant(nullptr);
        }
        zombies_.erase(zombies_.begin() + static_cast<long>(i));
    }
}

void Game::removeDeadHumans() {
    for (size_t i = 0; i < humans_.size();) {
        Human& human = *humans_[i];
        if (human.alive()) {
            ++i;
            continue;
        }
        if (grid_.isInside(human.row(), human.col()) &&
            grid_.cell(human.row(), human.col()).occupant() == &human) {
            grid_.cell(human.row(), human.col()).setOccupant(nullptr);
        }
        log(QStringLiteral("A survivor died."));
        humans_.erase(humans_.begin() + static_cast<long>(i));
    }
}

void Game::spawnWave() {
    if (turn_ == 0 || turn_ % WaveInterval != 0 || turn_ > MaxTurns) {
        return;
    }

    const int count = 3 + (turn_ >= 24 ? 2 : 0);
    int placed = 0;

    for (int attempt = 0; attempt < 300 && placed < count; ++attempt) {
        int row = 0;
        int col = 0;
        switch (randomInt(0, 3)) {
        case 0:
            row = 0;
            col = randomInt(1, grid_.cols() - 2);
            break;
        case 1:
            row = grid_.rows() - 1;
            col = randomInt(1, grid_.cols() - 2);
            break;
        case 2:
            row = randomInt(1, grid_.rows() - 2);
            col = 0;
            break;
        default:
            row = randomInt(1, grid_.rows() - 2);
            col = grid_.cols() - 1;
            break;
        }

        const auto type = static_cast<ZombieType>(randomInt(0, 2));
        if (spawnZombie(type, row, col)) {
            ++placed;
        }
    }

    if (placed > 0) {
        log(QStringLiteral("A new wave arrives from the dark."));
    }
}

int Game::manhattan(int rowA, int colA, int rowB, int colB) {
    return manhattanStatic(rowA, colA, rowB, colB);
}

int Game::manhattan(const Entity& a, const Entity& b) {
    return manhattanStatic(a.row(), a.col(), b.row(), b.col());
}

bool Game::tryStep(Entity& entity, int dr, int dc) {
    if (dr == 0 && dc == 0) {
        return false;
    }
    const int row = entity.row() + dr;
    const int col = entity.col() + dc;
    if (!grid_.isFree(row, col)) {
        return false;
    }
    return grid_.moveEntity(entity, row, col);
}

bool Game::stepToward(Entity& entity, int row, int col) {
    const int dr = sign(row - entity.row());
    const int dc = sign(col - entity.col());

    if (dr != 0 && tryStep(entity, dr, 0)) {
        return true;
    }
    if (dc != 0 && tryStep(entity, 0, dc)) {
        return true;
    }
    return false;
}

bool Game::flee(Human& human) {
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    int bestRow = -1;
    int bestCol = -1;
    int bestScore = -1;

    for (int i = 0; i < 4; ++i) {
        const int row = human.row() + dr[i];
        const int col = human.col() + dc[i];
        if (!grid_.isFree(row, col)) {
            continue;
        }

        int score = 999;
        for (const auto& zombie : zombies_) {
            if (!zombie->alive()) {
                continue;
            }
            score = std::min(
                score, manhattan(row, col, zombie->row(), zombie->col()));
        }
        if (score > bestScore) {
            bestScore = score;
            bestRow = row;
            bestCol = col;
        }
    }

    if (bestRow < 0) {
        return false;
    }
    return grid_.moveEntity(human, bestRow, bestCol);
}

bool Game::wander(Entity& entity) {
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    int rows[4];
    int cols[4];
    int count = 0;

    for (int i = 0; i < 4; ++i) {
        const int row = entity.row() + dr[i];
        const int col = entity.col() + dc[i];
        if (grid_.isFree(row, col)) {
            rows[count] = row;
            cols[count] = col;
            ++count;
        }
    }

    if (count == 0) {
        return false;
    }
    const int pick = randomInt(0, count - 1);
    return grid_.moveEntity(entity, rows[pick], cols[pick]);
}

Human* Game::nearestHuman(const Entity& from) {
    Human* best = nullptr;
    int bestDistance = 999;
    for (auto& human : humans_) {
        if (!human->alive()) {
            continue;
        }
        const int distance = manhattan(from, *human);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = human.get();
        }
    }
    return best;
}

Zombie* Game::nearestZombie(const Entity& from, int maxRange) {
    Zombie* best = nullptr;
    int bestDistance = maxRange + 1;
    for (auto& zombie : zombies_) {
        if (!zombie->alive()) {
            continue;
        }
        const int distance = manhattan(from, *zombie);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = zombie.get();
        }
    }
    return best;
}

Zombie* Game::adjacentZombie(const Entity& from) {
    for (auto& zombie : zombies_) {
        if (zombie->alive() && manhattan(from, *zombie) <= 1) {
            return zombie.get();
        }
    }
    return nullptr;
}

Human* Game::nearestInfected(const Human& from) {
    Human* best = nullptr;
    int bestDistance = 999;
    for (auto& other : humans_) {
        if (other.get() == &from || !other->alive() ||
            other->contaminationTimer() == 0) {
            continue;
        }
        const int distance = manhattan(from, *other);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = other.get();
        }
    }
    return best;
}

Human* Game::adjacentInfected(const Human& from) {
    for (auto& other : humans_) {
        if (other.get() == &from || !other->alive()) {
            continue;
        }
        if (other->contaminationTimer() > 0 &&
            manhattan(from, *other) <= 1) {
            return other.get();
        }
    }
    return nullptr;
}

Plant* Game::ripePlantNear(const Human& human) {
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            const int row = human.row() + dr;
            const int col = human.col() + dc;
            if (!grid_.isInside(row, col)) {
                continue;
            }
            Plant* plant = grid_.cell(row, col).plant();
            if (plant && plant->isFullyGrown()) {
                return plant;
            }
        }
    }
    return nullptr;
}

Plant* Game::nearestRipePlant(const Human& human, int maxDistance) {
    Plant* best = nullptr;
    int bestDistance = maxDistance + 1;
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            Plant* plant = grid_.cell(row, col).plant();
            if (!plant || !plant->isFullyGrown()) {
                continue;
            }
            const int distance = manhattan(human.row(), human.col(), row, col);
            if (distance < bestDistance) {
                bestDistance = distance;
                best = plant;
            }
        }
    }
    return best;
}

Wall* Game::damagedWallNear(const Human& human) {
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            const int row = human.row() + dr;
            const int col = human.col() + dc;
            if (!grid_.isInside(row, col)) {
                continue;
            }
            Wall* wall = grid_.cell(row, col).wall();
            if (wall && !wall->isDestroyed() &&
                wall->durability() < wall->maxDurability()) {
                return wall;
            }
        }
    }
    return nullptr;
}

Wall* Game::nearestDamagedWall(const Human& human, int maxDistance) {
    Wall* best = nullptr;
    int bestDistance = maxDistance + 1;
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            Wall* wall = grid_.cell(row, col).wall();
            if (!wall || wall->isDestroyed() ||
                wall->durability() >= wall->maxDurability()) {
                continue;
            }
            const int distance = manhattan(human.row(), human.col(), row, col);
            if (distance < bestDistance) {
                bestDistance = distance;
                best = wall;
            }
        }
    }
    return best;
}

void Game::damageZombie(Zombie& zombie, int amount) {
    if (!zombie.alive()) {
        return;
    }
    zombie.takeDamage(amount);
    if (zombie.alive()) {
        return;
    }
    if (grid_.isInside(zombie.row(), zombie.col()) &&
        grid_.cell(zombie.row(), zombie.col()).occupant() == &zombie) {
        grid_.cell(zombie.row(), zombie.col()).setOccupant(nullptr);
    }
    log(zombie.typeName() + QStringLiteral(" was destroyed."));
}

void Game::explodeZombie(Zombie& zombie) {
    for (auto& human : humans_) {
        if (human->alive() && manhattan(zombie, *human) <= 2) {
            human->getInfected(2);
            human->takeDamage(3);
            addBlood(human->row(), human->col());
        }
    }
    log(QStringLiteral("An exploder bursts! Nearby survivors are hit."));
    damageZombie(zombie, zombie.maxHealth());
}

void Game::addFood(int amount) {
    food_ = std::max(0, food_ + amount);
}

void Game::addBlood(int row, int col) {
    const QPoint spot(col, row);
    if (std::find(blood_.begin(), blood_.end(), spot) == blood_.end()) {
        blood_.push_back(spot);
    }
}

void Game::log(const QString& message) {
    log_.push_back(message);
    while (log_.size() > 8) {
        log_.removeFirst();
    }
}

QString Game::lastEvent() const {
    if (log_.isEmpty()) {
        return QStringLiteral("Stay quiet. Listen for the horde.");
    }
    return log_.last();
}

int Game::randomInt(int low, int high) {
    std::uniform_int_distribution<int> distribution(low, high);
    return distribution(rng_);
}

int Game::humanCount() const {
    int count = 0;
    for (const auto& human : humans_) {
        if (human->alive()) {
            ++count;
        }
    }
    return count;
}

int Game::zombieCount() const {
    int count = 0;
    for (const auto& zombie : zombies_) {
        if (zombie->alive()) {
            ++count;
        }
    }
    return count;
}

int Game::infectedCount() const {
    int count = 0;
    for (const auto& human : humans_) {
        if (human->alive() && human->contaminationTimer() > 0) {
            ++count;
        }
    }
    return count;
}

int Game::defense() const {
    int total = 0;
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            const Wall* wall = grid_.cell(row, col).wall();
            if (wall && !wall->isDestroyed()) {
                total += wall->durability();
            }
        }
    }
    return total;
}

int Game::wallCount() const {
    int count = 0;
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            const Wall* wall = grid_.cell(row, col).wall();
            if (wall && !wall->isDestroyed()) {
                ++count;
            }
        }
    }
    return count;
}

QString Game::weatherName() const {
    switch (weather_) {
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

qreal Game::rainIntensity() const {
    switch (weather_) {
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

QString Game::resultText() const {
    if (state_ == GameState::NotStarted) {
        return QStringLiteral("READY");
    }
    if (state_ == GameState::Ongoing) {
        return QStringLiteral("SURVIVE");
    }
    return humansWon_ ? QStringLiteral("THE SURVIVORS HELD OUT")
                      : QStringLiteral("THE HORDE TOOK THE CAMP");
}

void Game::checkEndCondition() {
    if (!isRunning()) {
        return;
    }
    if (humans_.empty()) {
        state_ = GameState::GameOver;
        humansWon_ = false;
        log(QStringLiteral("The horde took the camp."));
    } else if (turn_ >= MaxTurns) {
        state_ = GameState::GameOver;
        humansWon_ = true;
        log(QStringLiteral("Dawn breaks. The survivors held out."));
    }
}
