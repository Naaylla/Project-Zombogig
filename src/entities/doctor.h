#pragma once

#include "entities/human.h"

class Doctor final : public Human {
public:
    Doctor(int id, int row, int col);
    void act(Game& game) override;
};
