#pragma once

#include <QString>

enum class PropKind {
    Wall,
    Bush,
    Tree,
    Crate,
    Barrel,
    Rock,
    Corpse,
    Campfire
};

class Prop {
public:
    explicit Prop(PropKind kind);

    PropKind kind() const { return kind_; }
    QString name() const;
    int hp() const;
    int defense() const;
    int food() const;
    int height() const;

private:
    PropKind kind_;
};
