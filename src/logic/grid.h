#pragma once

#include <vector>

#include "logic/cell.h"

class Entity;

class Grid {
public:
    Grid(int rows = 12, int cols = 12);

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    Cell& cell(int row, int col);
    const Cell& cell(int row, int col) const;
    bool isInside(int row, int col) const;
    bool isFree(int row, int col) const;

    bool moveEntity(Entity& entity, int row, int col);
    void reset();

private:
    int index(int row, int col) const { return row * cols_ + col; }

    int rows_;
    int cols_;
    std::vector<Cell> cells_;
};
