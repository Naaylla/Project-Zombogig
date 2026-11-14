#include <algorithm>

#include "entities/entity.h"

Entity::Entity(int id, int maxHealth, int attack, int speed, int row,
               int col)
    : id_(id),
      maxHealth_(maxHealth),
      health_(maxHealth),
      attack_(attack),
      speed_(speed),
      row_(row),
      col_(col) {}

void Entity::takeDamage(int amount) {
    health_ = std::max(0, health_ - amount);
}

void Entity::heal(int amount) {
    health_ = std::min(maxHealth_, health_ + amount);
}
