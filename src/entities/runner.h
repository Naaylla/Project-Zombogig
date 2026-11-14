#pragma once

#include "entities/zombie.h"

class Runner final : public Zombie {
public:
    Runner(int id, int row, int col);

protected:
    int steps() const override;
};
