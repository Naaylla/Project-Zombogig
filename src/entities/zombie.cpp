#include "entities/zombie.h"

#include "entities/exploder.h"
#include "entities/human.h"
#include "entities/runner.h"
#include "entities/tank.h"
#include "logic/game.h"
#include "logic/wall.h"

namespace {

ZombieStats zombieStats(ZombieType type) {
    switch (type) {
    case ZombieType::Tank:
        return {22, 3, 1};
    case ZombieType::Runner:
        return {8, 2, 2};
    case ZombieType::Exploder:
        return {11, 2, 1};
    }
    return {8, 2, 1};
}

QString zombieName(ZombieType type) {
    switch (type) {
    case ZombieType::Tank:
        return QStringLiteral("Tank Zombie");
    case ZombieType::Runner:
        return QStringLiteral("Runner Zombie");
    case ZombieType::Exploder:
        return QStringLiteral("Exploder Zombie");
    }
    return QStringLiteral("Zombie");
}

QColor zombieColor(ZombieType type) {
    switch (type) {
    case ZombieType::Tank:
        return QColor(66, 78, 62);
    case ZombieType::Runner:
        return QColor(110, 116, 88);
    case ZombieType::Exploder:
        return QColor(92, 104, 74);
    }
    return QColor(90, 100, 78);
}

Sprite zombieSprite(ZombieType type) {
    switch (type) {
    case ZombieType::Tank:
        return Sprite::Tank;
    case ZombieType::Runner:
        return Sprite::Runner;
    case ZombieType::Exploder:
        return Sprite::Exploder;
    }
    return Sprite::Tank;
}

}  // namespace

Zombie::Zombie(int id, ZombieType type, int row, int col)
    : Entity(id, zombieStats(type).health, zombieStats(type).attack,
             zombieStats(type).speed, row, col),
      type_(type) {}

QString Zombie::typeName() const {
    return zombieName(type_);
}

QColor Zombie::color() const {
    return zombieColor(type_);
}

Sprite Zombie::sprite() const {
    return zombieSprite(type_);
}

void Zombie::update(Game& game) {
    if (!alive()) {
        return;
    }

    Human* target = game.nearestHuman(*this);
    if (!target) {
        game.wander(*this);
        return;
    }

    if (explodesAtRange(Game::manhattan(*this, *target))) {
        explode(game);
        return;
    }

    for (int step = 0; step < steps(); ++step) {
        if (!target->alive() ||
            Game::manhattan(*this, *target) <= 1) {
            break;
        }
        if (game.stepToward(*this, target->row(), target->col())) {
            continue;
        }

        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};
        for (int i = 0; i < 4; ++i) {
            const int row = this->row() + dr[i];
            const int col = this->col() + dc[i];
            if (!game.grid().isInside(row, col)) {
                continue;
            }
            Wall* wall = game.grid().cell(row, col).wall();
            if (wall && !wall->isDestroyed()) {
                wall->takeDamage(attack());
                game.log(typeName() + QStringLiteral(" hits a wall."));
                break;
            }
        }
        break;
    }

    if (target->alive() && Game::manhattan(*this, *target) <= 1) {
        target->takeDamage(attack());
        game.addBlood(target->row(), target->col());
        game.log(typeName() + QStringLiteral(" wounds a survivor."));
    }
}

void Zombie::explode(Game& game) {
    game.explodeZombie(*this);
}

std::unique_ptr<Zombie> makeZombie(int id, ZombieType type, int row, int col) {
    switch (type) {
    case ZombieType::Tank:
        return std::make_unique<Tank>(id, row, col);
    case ZombieType::Runner:
        return std::make_unique<Runner>(id, row, col);
    case ZombieType::Exploder:
        return std::make_unique<Exploder>(id, row, col);
    }
    return std::make_unique<Tank>(id, row, col);
}
