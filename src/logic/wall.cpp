#include <algorithm>

#include "logic/wall.h"

Wall::Wall(int row, int col, int durability)
    : row_(row),
      col_(col),
      durability_(durability),
      maxDurability_(durability) {}

void Wall::takeDamage(int amount) {
    durability_ = std::max(0, durability_ - amount);
}

void Wall::repair(int amount) {
    durability_ = std::min(maxDurability_, durability_ + amount);
}
