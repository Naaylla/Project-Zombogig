#include "entities/human.h"

namespace {

struct Stats {
    int hp;
    int attack;
    int defense;
    int speed;
};

Stats statsFor(HumanType type) {
    switch (type) {
    case HumanType::Survivor:
        return {12, 3, 3, 3};
    case HumanType::Soldier:
        return {16, 6, 6, 2};
    case HumanType::Doctor:
        return {9, 1, 2, 3};
    case HumanType::Engineer:
        return {11, 2, 4, 2};
    }
    return {10, 2, 2, 2};
}

QColor colorFor(HumanType type) {
    switch (type) {
    case HumanType::Survivor:
        return QColor(122, 104, 80);
    case HumanType::Soldier:
        return QColor(84, 92, 66);
    case HumanType::Doctor:
        return QColor(150, 148, 140);
    case HumanType::Engineer:
        return QColor(130, 92, 56);
    }
    return QColor(120, 110, 90);
}

QString nameFor(HumanType type) {
    switch (type) {
    case HumanType::Survivor:
        return QStringLiteral("Survivor");
    case HumanType::Soldier:
        return QStringLiteral("Soldier");
    case HumanType::Doctor:
        return QStringLiteral("Doctor");
    case HumanType::Engineer:
        return QStringLiteral("Engineer");
    }
    return QStringLiteral("Human");
}

}  // namespace

Human::Human(HumanType type)
    : Entity(Faction::Human,
             statsFor(type).hp,
             statsFor(type).attack,
             statsFor(type).defense,
             statsFor(type).speed),
      type_(type) {}

QString Human::typeName() const {
    return nameFor(type_);
}

QColor Human::color() const {
    return colorFor(type_);
}
