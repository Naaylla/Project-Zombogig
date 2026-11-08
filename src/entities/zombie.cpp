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
    case ZombieType::Walker:
        return {10, 3, 1, 2};
    case ZombieType::Runner:
        return {7, 2, 0, 5};
    case ZombieType::Brute:
        return {24, 7, 4, 1};
    case ZombieType::Crawler:
        return {5, 2, 0, 1};
    }
    return {8, 2, 1, 2};
}

QColor colorFor(ZombieType type) {
    switch (type) {
    case ZombieType::Walker:
        return QColor(86, 102, 76);
    case ZombieType::Runner:
        return QColor(110, 116, 88);
    case ZombieType::Brute:
        return QColor(66, 78, 62);
    case ZombieType::Crawler:
        return QColor(92, 104, 74);
    }
    return QColor(90, 100, 78);
}

QString nameFor(ZombieType type) {
    switch (type) {
    case ZombieType::Walker:
        return QStringLiteral("Walker Zombie");
    case ZombieType::Runner:
        return QStringLiteral("Runner Zombie");
    case ZombieType::Brute:
        return QStringLiteral("Brute Zombie");
    case ZombieType::Crawler:
        return QStringLiteral("Crawler Zombie");
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
