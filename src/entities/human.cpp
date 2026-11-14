#include "entities/human.h"

#include <algorithm>

#include "entities/builder.h"
#include "entities/doctor.h"
#include "entities/farmer.h"
#include "entities/military.h"
#include "logic/game.h"

namespace {

HumanStats humanStats(HumanType type) {
    switch (type) {
    case HumanType::Farmer:
        return {14, 4, 1};
    case HumanType::Doctor:
        return {12, 3, 1};
    case HumanType::Military:
        return {20, 11, 1};
    case HumanType::Builder:
        return {16, 5, 1};
    }
    return {14, 4, 1};
}

QString humanName(HumanType type) {
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
    return QStringLiteral("Survivor");
}

QColor humanColor(HumanType type) {
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

Sprite humanSprite(HumanType type) {
    switch (type) {
    case HumanType::Farmer:
        return Sprite::Farmer;
    case HumanType::Doctor:
        return Sprite::Doctor;
    case HumanType::Military:
        return Sprite::Military;
    case HumanType::Builder:
        return Sprite::Builder;
    }
    return Sprite::Farmer;
}

}  // namespace

Human::Human(int id, HumanType type, int row, int col)
    : Entity(id, humanStats(type).health, humanStats(type).attack,
             humanStats(type).speed, row, col),
      type_(type) {}

QString Human::typeName() const {
    return humanName(type_);
}

QColor Human::color() const {
    return humanColor(type_);
}

Sprite Human::sprite() const {
    return humanSprite(type_);
}

void Human::update(Game& game) {
    if (!alive()) {
        return;
    }
    if (Zombie* threat = game.adjacentZombie(*this)) {
        game.damageZombie(*threat, attack());
        game.flee(*this);
        return;
    }

    act(game);

    if (game.randomInt(0, 99) < 15) {
        game.wander(*this);
    }
}

void Human::getInfected(int amount) {
    if (status_ == Status::Transformed) {
        return;
    }
    contaminationTimer_ =
        std::min(InfectionTurns, contaminationTimer_ + amount);
    if (contaminationTimer_ > 0) {
        status_ = Status::Infected;
    }
}

void Human::cure() {
    contaminationTimer_ = 0;
    status_ = Status::Healthy;
    heal(2);
}

void Human::loseHunger() {
    hunger_ = std::max(0, hunger_ - 7);
    if (hunger_ == 0) {
        takeDamage(2);
    }
}

void Human::eat() {
    hunger_ = std::min(100, hunger_ + 45);
}

std::unique_ptr<Human> makeHuman(int id, HumanType type, int row, int col) {
    switch (type) {
    case HumanType::Farmer:
        return std::make_unique<Farmer>(id, row, col);
    case HumanType::Doctor:
        return std::make_unique<Doctor>(id, row, col);
    case HumanType::Military:
        return std::make_unique<Military>(id, row, col);
    case HumanType::Builder:
        return std::make_unique<Builder>(id, row, col);
    }
    return std::make_unique<Farmer>(id, row, col);
}
