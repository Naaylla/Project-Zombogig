#include "logic/grid.h"

#include "entities/entity.h"

Grid::Grid(int rows, int cols)
    : rows_(rows), cols_(cols) {
    cells_.reserve(static_cast<size_t>(rows_ * cols_));
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            cells_.emplace_back(row, col);
        }
    }
}

Cell& Grid::cell(int row, int col) {
    return cells_[index(row, col)];
}

const Cell& Grid::cell(int row, int col) const {
    return cells_[index(row, col)];
}

bool Grid::isInside(int row, int col) const {
    return row >= 0 && col >= 0 && row < rows_ && col < cols_;
}

bool Grid::isFree(int row, int col) const {
    if (!isInside(row, col)) {
        return false;
    }
    const Cell& target = cell(row, col);
    return target.isWalkable() && target.isEmpty();
}

bool Grid::moveEntity(Entity& entity, int row, int col) {
    if (!isInside(row, col)) {
        return false;
    }
    Cell& target = cell(row, col);
    if (!target.isEmpty() && target.occupant() != &entity) {
        return false;
    }
    if (isInside(entity.row(), entity.col())) {
        Cell& source = cell(entity.row(), entity.col());
        if (source.occupant() == &entity) {
            source.setOccupant(nullptr);
        }
    }
    target.setOccupant(&entity);
    entity.setPosition(row, col);
    return true;
}

void Grid::reset() {
    for (Cell& current : cells_) {
        current.reset();
    }
}
