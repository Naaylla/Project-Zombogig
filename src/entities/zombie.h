#pragma once

#include "entities/entity.h"

enum class ZombieType {
    Tank,
    Runner,
    Exploder
};

class Zombie : public Entity {
public:
    explicit Zombie(ZombieType type);

    ZombieType type() const { return type_; }
    QString typeName() const override;
    QColor color() const override;

private:
    ZombieType type_;
};
