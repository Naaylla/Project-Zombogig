#pragma once

class Plant {
public:
    static constexpr int MaxGrowth = 4;

    Plant(int row, int col);

    int row() const { return row_; }
    int col() const { return col_; }
    int growth() const { return growth_; }
    bool isFullyGrown() const { return growth_ >= MaxGrowth; }

    void grow();
    int harvest();
    void reset() { growth_ = 0; }

private:
    int row_;
    int col_;
    int growth_ = 0;
};
