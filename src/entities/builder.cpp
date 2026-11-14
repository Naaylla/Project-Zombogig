#include "entities/builder.h"

#include "logic/game.h"
#include "logic/wall.h"

Builder::Builder(int id, int row, int col)
    : Human(id, HumanType::Builder, row, col) {}

void Builder::act(Game& game) {
    if (Wall* wall = game.damagedWallNear(*this)) {
        wall->repair(4);
        game.log(QStringLiteral("A builder repaired a wall."));
        return;
    }
    if (Wall* wall = game.nearestDamagedWall(*this, 7)) {
        if (!game.stepToward(*this, wall->row(), wall->col())) {
            game.wander(*this);
        }
        return;
    }
    game.wander(*this);
}
