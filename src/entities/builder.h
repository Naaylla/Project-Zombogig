#pragma once

#include "entities/human.h"

class Builder final : public Human {
public:
    Builder(int id, int row, int col);
    void act(Game& game) override;
};
