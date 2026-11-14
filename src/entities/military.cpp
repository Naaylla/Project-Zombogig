#include "entities/military.h"

#include "entities/zombie.h"
#include "logic/game.h"

Military::Military(int id, int row, int col)
    : Human(id, HumanType::Military, row, col) {}

void Military::act(Game& game) {
    if (cooldown_ > 0) {
        --cooldown_;
    }
    if (cooldown_ == 0 && game.nearestZombie(*this, attackRange_)) {
        shoot(game);
        return;
    }
    if (!game.nearestZombie(*this, attackRange_)) {
        game.wander(*this);
    }
}

void Military::shoot(Game& game) {
    Zombie* target = game.nearestZombie(*this, attackRange_);
    if (!target) {
        return;
    }
    game.damageZombie(*target, attack());
    cooldown_ = 1;
    game.log(QStringLiteral("The military shot a zombie."));
}
