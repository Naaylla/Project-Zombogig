#include "prop.h"

namespace {

struct Stats {
    int hp;
    int defense;
    int food;
    int height;
};

Stats statsFor(PropKind kind) {
    switch (kind) {
    case PropKind::Wall:
        return {30, 5, 0, 46};
    case PropKind::Bush:
        return {5, 0, 4, 12};
    case PropKind::Tree:
        return {12, 0, 1, 54};
    case PropKind::Plant:
        return {4, 0, 1, 8};
    case PropKind::Corpse:
        return {0, 0, 0, 2};
    case PropKind::Campfire:
        return {10, 0, 0, 18};
    }
    return {0, 0, 0, 0};
}

QString nameFor(PropKind kind) {
    switch (kind) {
    case PropKind::Wall:
        return QStringLiteral("Wooden Wall");
    case PropKind::Bush:
        return QStringLiteral("Berry Bush");
    case PropKind::Tree:
        return QStringLiteral("Dead Tree");
    case PropKind::Plant:
        return QStringLiteral("Food Plant");
    case PropKind::Corpse:
        return QStringLiteral("Corpse");
    case PropKind::Campfire:
        return QStringLiteral("Campfire");
    }
    return QStringLiteral("Object");
}

}  // namespace

Prop::Prop(PropKind kind)
    : kind_(kind), growth_(0) {}

QString Prop::name() const {
    return nameFor(kind_);
}

int Prop::hp() const {
    return statsFor(kind_).hp;
}

int Prop::defense() const {
    return statsFor(kind_).defense;
}

int Prop::food() const {
    if (kind_ == PropKind::Plant) {
        return statsFor(kind_).food + growth_;
    }
    return statsFor(kind_).food;
}

int Prop::height() const {
    return statsFor(kind_).height;
}

bool Prop::isFullyGrown() const {
    return kind_ != PropKind::Plant || growth_ >= 3;
}

void Prop::grow() {
    if (kind_ == PropKind::Plant && growth_ < 3) {
        ++growth_;
    }
}
