#include "entities/doctor.h"

#include "logic/game.h"

Doctor::Doctor(int id, int row, int col)
    : Human(id, HumanType::Doctor, row, col) {}

void Doctor::act(Game& game) {
    if (Human* patient = game.adjacentInfected(*this)) {
        patient->cure();
        game.log(QStringLiteral("A doctor cured a survivor."));
        return;
    }
    if (Human* patient = game.nearestInfected(*this)) {
        if (!game.stepToward(*this, patient->row(), patient->col())) {
            game.wander(*this);
        }
        return;
    }
    game.wander(*this);
}
