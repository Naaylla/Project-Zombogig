#include "logic/cell.h"

Cell::Cell(int row, int col)
    : row_(row), col_(col) {}

bool Cell::isWalkable() const {
    if (wall_ && !wall_->isDestroyed()) {
        return false;
    }
    switch (decoration_) {
    case Decoration::Tree:
    case Decoration::Bush:
    case Decoration::Campfire:
        return false;
    case Decoration::None:
    case Decoration::Corpse:
        return true;
    }
    return true;
}

void Cell::reset() {
    occupant_ = nullptr;
    wall_.reset();
    plant_.reset();
    decoration_ = Decoration::None;
}
