#pragma once

#include <QString>

enum class PropKind {
    Wall,
    Bush,
    Tree,
    Plant,
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
    int growth() const { return growth_; }
    bool isFullyGrown() const;
    void grow();

private:
    PropKind kind_;
    int growth_;
};
