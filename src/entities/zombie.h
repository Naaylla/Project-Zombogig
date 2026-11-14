#pragma once

#include <memory>

#include "entities/entity.h"

struct ZombieStats {
    int health;
    int attack;
    int speed;
};

class Zombie : public Entity {
public:
    Zombie(int id, ZombieType type, int row, int col);

    ZombieType type() const { return type_; }
    QString typeName() const override;
    QColor color() const override;
    Sprite sprite() const override;
    void update(Game& game) override;

protected:
    virtual int steps() const { return 1; }
    virtual bool explodesAtRange(int) const { return false; }
    virtual void explode(Game& game);

    ZombieType type_;
};

std::unique_ptr<Zombie> makeZombie(int id, ZombieType type, int row, int col);
