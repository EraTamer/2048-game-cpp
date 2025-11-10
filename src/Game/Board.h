#ifndef BOARD_H
#define BOARD_H

#include "../Utils/Constants.h"
#include <vector>

class Board {
private:
    std::vector<std::vector<int>> grid;
    int score;

public:
    Board();
    void initialize();
    void addRandomTile();
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();
    bool isGameOver() const;
    bool hasWon() const;

    // Getters
    const std::vector<std::vector<int>>& getGrid() const { return grid; }
    int getScore() const { return score; }

private:
    bool canMove() const;
    bool compressRow(std::vector<int>& row);
    bool mergeRow(std::vector<int>& row);
};

#endif