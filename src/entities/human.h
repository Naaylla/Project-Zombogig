#pragma once

#include "entities/entity.h"

enum class HumanType {
    Survivor,
    Soldier,
    Doctor,
    Engineer
};

class Human : public Entity {
public:
    explicit Human(HumanType type);

    HumanType type() const { return type_; }
    QString typeName() const override;
    QColor color() const override;

private:
    HumanType type_;
};
