#include "entities/runner.h"

Runner::Runner(int id, int row, int col)
    : Zombie(id, ZombieType::Runner, row, col) {}

int Runner::steps() const {
    return 2;
}
