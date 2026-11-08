#include "wall.h"

Wall::Wall(int row, int col)
    : Cell(row, col, CellType::Wall) {}

Color Wall::color() const {
    return Color{86, 86, 92};
}
