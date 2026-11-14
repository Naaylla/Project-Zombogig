#include <algorithm>

#include "logic/plant.h"

Plant::Plant(int row, int col)
    : row_(row), col_(col) {}

void Plant::grow() {
    growth_ = std::min(MaxGrowth, growth_ + 1);
}

int Plant::harvest() {
    const int food = 2 + growth_;
    growth_ = 0;
    return food;
}
