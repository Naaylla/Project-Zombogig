#pragma once

#include "entities/zombie.h"

class Tank final : public Zombie {
public:
    Tank(int id, int row, int col);
};
