#pragma once

#include <memory>
#include <random>
#include <vector>

#include <QPoint>
#include <QString>
#include <QStringList>

#include "entities/human.h"
#include "entities/zombie.h"
#include "logic/grid.h"
#include "logic/types.h"

class Game {
public:
    static constexpr int MaxTurns = 48;
    static constexpr int WaveInterval = 8;

    Game();

    void start(int survivors, int zombies, unsigned seed);
    void nextTurn();
    void reset();

    GameState state() const { return state_; }
    bool isRunning() const { return state_ == GameState::Ongoing; }
    bool isOver() const { return state_ == GameState::GameOver; }
    bool humansWon() const { return humansWon_; }
    QString resultText() const;

    int turn() const { return turn_; }
    int day() const { return turn_ / 24 + 1; }
    int minutesOfDay() const { return (7 * 60 + turn_ * 60) % (24 * 60); }
    int hour() const { return minutesOfDay() / 60; }
    int turnsLeft() const { return turn_ >= MaxTurns ? 0 : MaxTurns - turn_; }
    int nextWaveIn() const {
        const int next = (turn_ / WaveInterval + 1) * WaveInterval;
        return next <= MaxTurns ? next - turn_ : 0;
    }

    int humanCount() const;
    int zombieCount() const;
    int infectedCount() const;
    int food() const { return food_; }
    int defense() const;
    int wallCount() const;

    Weather weather() const { return weather_; }
    QString weatherName() const;
    qreal rainIntensity() const;

    const QStringList& log() const { return log_; }
    QString lastEvent() const;
    const std::vector<QPoint>& blood() const { return blood_; }

    Grid& grid() { return grid_; }
    const Grid& grid() const { return grid_; }

    static int manhattan(int rowA, int colA, int rowB, int colB);
    static int manhattan(const Entity& a, const Entity& b);

    bool tryStep(Entity& entity, int dr, int dc);
    bool stepToward(Entity& entity, int row, int col);
    bool flee(Human& human);
    bool wander(Entity& entity);

    Human* nearestHuman(const Entity& from);
    Zombie* nearestZombie(const Entity& from, int maxRange);
    Zombie* adjacentZombie(const Entity& from);
    Human* nearestInfected(const Human& from);
    Human* adjacentInfected(const Human& from);
    Plant* ripePlantNear(const Human& human);
    Plant* nearestRipePlant(const Human& human, int maxDistance);
    Wall* damagedWallNear(const Human& human);
    Wall* nearestDamagedWall(const Human& human, int maxDistance);

    void damageZombie(Zombie& zombie, int amount);
    void explodeZombie(Zombie& zombie);
    void addFood(int amount);
    void addBlood(int row, int col);
    void log(const QString& message);
    int randomInt(int low, int high);

private:
    void buildMap();
    void spawnInitialHumans(int count);
    void spawnInitialZombies(int count);
    void spawnWave();
    bool spawnHuman(HumanType type, int row, int col);
    bool spawnZombie(ZombieType type, int row, int col);

    void updateWeather();
    void updateFood();
    void growPlants();

    void moveZombies();
    void actHumans();
    void infectHumans();
    void transformHumans();
    void removeDeadZombies();
    void removeDeadHumans();
    void checkEndCondition();

    Grid grid_;
    std::vector<std::unique_ptr<Human>> humans_;
    std::vector<std::unique_ptr<Zombie>> zombies_;
    std::vector<QPoint> blood_;
    QStringList log_;
    std::mt19937 rng_;

    unsigned seed_ = 0;
    int nextId_ = 1;
    int turn_ = 0;
    int food_ = 14;
    GameState state_ = GameState::NotStarted;
    Weather weather_ = Weather::LightRain;
    bool humansWon_ = false;
};
