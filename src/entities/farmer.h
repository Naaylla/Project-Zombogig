#pragma once

#include "entities/human.h"

class Farmer final : public Human {
public:
    Farmer(int id, int row, int col);
    void act(Game& game) override;
};
