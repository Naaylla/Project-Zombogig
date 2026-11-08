#pragma once

#include "entities/entity.h"

enum class HumanType {
    Farmer,
    Doctor,
    Military,
    Builder
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
