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
    case HumanType::Farmer:
        return {10, 2, 2, 3};
    case HumanType::Doctor:
        return {9, 1, 2, 3};
    case HumanType::Military:
        return {16, 6, 6, 2};
    case HumanType::Builder:
        return {13, 3, 5, 2};
    }
    return {10, 2, 2, 2};
}

QColor colorFor(HumanType type) {
    switch (type) {
    case HumanType::Farmer:
        return QColor(122, 104, 80);
    case HumanType::Doctor:
        return QColor(150, 148, 140);
    case HumanType::Military:
        return QColor(84, 92, 66);
    case HumanType::Builder:
        return QColor(130, 92, 56);
    }
    return QColor(120, 110, 90);
}

QString nameFor(HumanType type) {
    switch (type) {
    case HumanType::Farmer:
        return QStringLiteral("Farmer");
    case HumanType::Doctor:
        return QStringLiteral("Doctor");
    case HumanType::Military:
        return QStringLiteral("Military");
    case HumanType::Builder:
        return QStringLiteral("Builder");
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
