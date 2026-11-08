#pragma once

#include <QColor>
#include <QString>

enum class Faction {
    Human,
    Zombie
};

class Entity {
public:
    Entity(Faction faction, int maxHp, int attack, int defense, int speed);
    virtual ~Entity() = default;

    Faction faction() const { return faction_; }
    virtual QString typeName() const = 0;
    virtual QColor color() const = 0;

    int maxHp() const { return maxHp_; }
    int attack() const { return attack_; }
    int defense() const { return defense_; }
    int speed() const { return speed_; }

protected:
    Faction faction_;
    int maxHp_;
    int attack_;
    int defense_;
    int speed_;
};
