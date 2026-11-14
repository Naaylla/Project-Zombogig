#pragma once

#include "entities/zombie.h"

class Exploder final : public Zombie {
public:
    Exploder(int id, int row, int col);

protected:
    bool explodesAtRange(int distance) const override;
    void explode(Game& game) override;
};
