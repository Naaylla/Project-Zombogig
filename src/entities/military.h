#pragma once

#include "entities/human.h"

class Military final : public Human {
public:
    Military(int id, int row, int col);
    void act(Game& game) override;

private:
    int attackRange_ = 4;
    int cooldown_ = 0;

    void shoot(Game& game);
};
