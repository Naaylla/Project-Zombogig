#pragma once

class Wall {
public:
    Wall(int row, int col, int durability = 10);

    int row() const { return row_; }
    int col() const { return col_; }
    int durability() const { return durability_; }
    int maxDurability() const { return maxDurability_; }
    bool isDestroyed() const { return durability_ <= 0; }

    void takeDamage(int amount);
    void repair(int amount);

private:
    int row_;
    int col_;
    int durability_;
    int maxDurability_;
};
