#include "entities/tank.h"

Tank::Tank(int id, int row, int col)
    : Zombie(id, ZombieType::Tank, row, col) {}
