#include "cell.h"

#include "entities/entity.h"
#include "prop.h"

Cell::Cell(int row, int col, CellType type)
    : row_(row), col_(col), type_(type), prop_(nullptr), entity_(nullptr) {}

Cell::~Cell() {
    delete prop_;
    delete entity_;
}

Color Cell::color() const {
    return Color{58, 72, 54};
}

void Cell::setProp(Prop* prop) {
    if (prop_ == prop) {
        return;
    }
    delete prop_;
    prop_ = prop;
}

void Cell::setEntity(Entity* entity) {
    if (entity_ == entity) {
        return;
    }
    delete entity_;
    entity_ = entity;
}
