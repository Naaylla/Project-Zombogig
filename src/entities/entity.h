#pragma once

#include <QColor>
#include <QString>

#include "logic/types.h"

class Game;

enum class Status {
    Healthy,
    Infected,
    Transformed
};

class Entity {
public:
    Entity(int id, int maxHealth, int attack, int speed, int row, int col);
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    int id() const { return id_; }
    int health() const { return health_; }
    int maxHealth() const { return maxHealth_; }
    int attack() const { return attack_; }
    int speed() const { return speed_; }
    bool alive() const { return health_ > 0; }

    int row() const { return row_; }
    int col() const { return col_; }
    void setPosition(int row, int col) {
        row_ = row;
        col_ = col;
    }

    virtual QString typeName() const = 0;
    virtual QColor color() const = 0;
    virtual Sprite sprite() const = 0;
    virtual void update(Game& game) = 0;

    void takeDamage(int amount);
    void heal(int amount);

private:
    int id_;
    int maxHealth_;
    int health_;
    int attack_;
    int speed_;
    int row_;
    int col_;
};
