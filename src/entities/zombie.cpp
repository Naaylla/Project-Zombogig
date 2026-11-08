#include "entities/zombie.h"

namespace {

struct Stats {
    int hp;
    int attack;
    int defense;
    int speed;
};

Stats statsFor(ZombieType type) {
    switch (type) {
    case ZombieType::Tank:
        return {24, 7, 4, 1};
    case ZombieType::Runner:
        return {7, 2, 0, 5};
    case ZombieType::Exploder:
        return {12, 5, 1, 2};
    }
    return {8, 2, 1, 2};
}

QColor colorFor(ZombieType type) {
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

QString nameFor(ZombieType type) {
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

}  // namespace

Zombie::Zombie(ZombieType type)
    : Entity(Faction::Zombie,
             statsFor(type).hp,
             statsFor(type).attack,
             statsFor(type).defense,
             statsFor(type).speed),
      type_(type) {}

QString Zombie::typeName() const {
    return nameFor(type_);
}

QColor Zombie::color() const {
    return colorFor(type_);
}
