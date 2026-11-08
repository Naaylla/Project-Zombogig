#pragma once

#include <vector>

#include "cell.h"

class Grid {
public:
    Grid(int rows, int cols);
    ~Grid();

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    Cell* at(int row, int col);
    const Cell* at(int row, int col) const;

    void setWall(int row, int col);

private:
    int rows_;
    int cols_;
    std::vector<std::vector<Cell*>> cells_;
};
