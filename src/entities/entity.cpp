#include "entities/entity.h"

Entity::Entity(Faction faction, int maxHp, int attack, int defense, int speed)
    : faction_(faction),
      maxHp_(maxHp),
      attack_(attack),
      defense_(defense),
      speed_(speed) {}
