#pragma once

#include "cell.h"

class Wall : public Cell {
public:
    Wall(int row, int col);
    Color color() const override;
};
