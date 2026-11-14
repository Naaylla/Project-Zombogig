#pragma once

#include <optional>

#include "logic/plant.h"
#include "logic/types.h"
#include "logic/wall.h"

class Entity;

class Cell {
public:
    Cell(int row, int col);

    int row() const { return row_; }
    int col() const { return col_; }

    Entity* occupant() const { return occupant_; }
    void setOccupant(Entity* occupant) { occupant_ = occupant; }
    bool isEmpty() const { return occupant_ == nullptr; }

    Wall* wall() { return wall_ ? &*wall_ : nullptr; }
    const Wall* wall() const { return wall_ ? &*wall_ : nullptr; }
    void buildWall(int durability = 10) {
        wall_.emplace(row_, col_, durability);
    }

    Plant* plant() { return plant_ ? &*plant_ : nullptr; }
    const Plant* plant() const { return plant_ ? &*plant_ : nullptr; }
    void seedPlant() { plant_.emplace(row_, col_); }

    Decoration decoration() const { return decoration_; }
    void setDecoration(Decoration decoration) { decoration_ = decoration; }

    bool isWalkable() const;
    void reset();

private:
    int row_;
    int col_;
    Entity* occupant_ = nullptr;
    std::optional<Wall> wall_;
    std::optional<Plant> plant_;
    Decoration decoration_ = Decoration::None;
};
