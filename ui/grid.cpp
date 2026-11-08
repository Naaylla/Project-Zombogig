#include "grid.h"

#include "wall.h"

Grid::Grid(int rows, int cols)
    : rows_(rows), cols_(cols) {
    cells_.resize(static_cast<size_t>(rows_));
    for (int row = 0; row < rows_; ++row) {
        cells_[row].reserve(static_cast<size_t>(cols_));
        for (int col = 0; col < cols_; ++col) {
            cells_[row].push_back(new Cell(row, col));
        }
    }
}

Grid::~Grid() {
    for (auto& row : cells_) {
        for (Cell* cell : row) {
            delete cell;
        }
    }
}

Cell* Grid::at(int row, int col) {
    return cells_[row][col];
}

const Cell* Grid::at(int row, int col) const {
    return cells_[row][col];
}

void Grid::setWall(int row, int col) {
    if (row < 0 || col < 0 || row >= rows_ || col >= cols_) {
        return;
    }
    delete cells_[row][col];
    cells_[row][col] = new Wall(row, col);
}
