#include "entities/farmer.h"

#include "logic/game.h"
#include "logic/plant.h"

Farmer::Farmer(int id, int row, int col)
    : Human(id, HumanType::Farmer, row, col) {}

void Farmer::act(Game& game) {
    if (Plant* plant = game.ripePlantNear(*this)) {
        game.addFood(plant->harvest());
        game.log(QStringLiteral("A farmer harvested food."));
        return;
    }
    if (Plant* plant = game.nearestRipePlant(*this, 7)) {
        if (!game.stepToward(*this, plant->row(), plant->col())) {
            game.wander(*this);
        }
        return;
    }
    game.wander(*this);
}
