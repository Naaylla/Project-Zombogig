#include "entities/exploder.h"

#include "logic/game.h"

Exploder::Exploder(int id, int row, int col)
    : Zombie(id, ZombieType::Exploder, row, col) {}

bool Exploder::explodesAtRange(int distance) const {
    return distance <= 1;
}

void Exploder::explode(Game& game) {
    game.explodeZombie(*this);
}
