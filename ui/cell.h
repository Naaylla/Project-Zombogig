#pragma once

enum class CellType {
    Floor,
    Wall
};

struct Color {
    int r;
    int g;
    int b;
};

class Entity;
class Prop;

class Cell {
public:
    Cell(int row, int col, CellType type = CellType::Floor);
    virtual ~Cell();

    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;

    int row() const { return row_; }
    int col() const { return col_; }
    CellType type() const { return type_; }
    bool walkable() const { return type_ == CellType::Floor; }

    virtual Color color() const;

    Prop* prop() const { return prop_; }
    void setProp(Prop* prop);

    Entity* entity() const { return entity_; }
    void setEntity(Entity* entity);

protected:
    int row_;
    int col_;
    CellType type_;
    Prop* prop_;
    Entity* entity_;
};
