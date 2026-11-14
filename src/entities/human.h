#pragma once

#include <memory>

#include "entities/entity.h"

struct HumanStats {
    int health;
    int attack;
    int speed;
};

class Human : public Entity {
public:
    Human(int id, HumanType type, int row, int col);

    HumanType type() const { return type_; }
    QString typeName() const override;
    QColor color() const override;
    Sprite sprite() const override;
    void update(Game& game) override;

    Status status() const { return status_; }
    int contaminationTimer() const { return contaminationTimer_; }
    int hunger() const { return hunger_; }
    bool shouldTransform() const {
        return contaminationTimer_ >= InfectionTurns;
    }
    bool wantsFood() const { return hunger_ <= 50; }

    void getInfected(int amount = 1);
    void cure();
    void loseHunger();
    void eat();

    static constexpr int InfectionTurns = 4;

protected:
    virtual void act(Game& game) = 0;

    HumanType type_;
    Status status_ = Status::Healthy;
    int contaminationTimer_ = 0;
    int hunger_ = 100;
};

std::unique_ptr<Human> makeHuman(int id, HumanType type, int row, int col);
