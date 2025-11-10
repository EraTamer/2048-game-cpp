#include "Board.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

Board::Board() : score(0) {
    std::srand(std::time(0)); // Seed for random number generation
    initialize();
}

void Board::initialize() {
    grid = std::vector<std::vector<int>>(Constants::BOARD_SIZE, 
                                        std::vector<int>(Constants::BOARD_SIZE, 0));
    score = 0;
    
    // Add initial tiles
    for (int i = 0; i < Constants::INITIAL_TILES; ++i) {
        addRandomTile();
    }
}

void Board::addRandomTile() {
    // Find all empty positions
    std::vector<std::pair<int, int>> emptyPositions;

    for (int i = 0; i < Constants::BOARD_SIZE; ++i) {
        for (int j = 0; j < Constants::BOARD_SIZE; ++j) {
            if (grid[i][j] == 0) {
                emptyPositions.emplace_back(i, j);
            }
        }
    }

    // If no empty positions, return
    if (emptyPositions.empty()) {
        return;
    }

    // Choose a random empty position
    int randomIndex = std::rand() % emptyPositions.size();
    auto [row, col] = emptyPositions[randomIndex];

    // 90% chance for 2, 10% chance for 4
    int value = (std::rand() % 10 == 0) ? 4 : 2;

    grid[row][col] = value;
}

bool Board::isGameOver() const {
    return !canMove();
}

bool Board::hasWon() const {
    for (const auto& row : grid) {
        for (int val : row) {
            if (val == Constants::WINNING_TILE) {
                return true;
            }
        }
    }
    return false;
}

bool Board::canMove() const {
    // Check for empty spaces
    for (const auto& row : grid) {
        for (int val : row) {
            if (val == 0) return true;
        }
    }

    // Check for possible merges horizontally
    for (int i = 0; i < Constants::BOARD_SIZE; ++i) {
        for (int j = 0; j < Constants::BOARD_SIZE - 1; ++j) {
            if (grid[i][j] == grid[i][j + 1]) return true;
        }
    }

    // Check for possible merges vertically
    for (int j = 0; j < Constants::BOARD_SIZE; ++j) {
        for (int i = 0; i < Constants::BOARD_SIZE - 1; ++i) {
            if (grid[i][j] == grid[i + 1][j]) return true;
        }
    }

    return false;
}

// Helper function to compress a row (move non-zero elements to the left)
bool Board::compressRow(std::vector<int>& row) {
    bool moved = false;
    int writeIndex = 0;

    // Move all non-zero elements to the left
    for (int readIndex = 0; readIndex < Constants::BOARD_SIZE; ++readIndex) {
        if (row[readIndex] != 0) {
            if (readIndex != writeIndex) {
                row[writeIndex] = row[readIndex];
                row[readIndex] = 0;
                moved = true;
            }
            writeIndex++;
        }
    }

    return moved;
}

// Helper function to merge adjacent equal tiles
bool Board::mergeRow(std::vector<int>& row) {
    bool merged = false;

    for (int i = 0; i < Constants::BOARD_SIZE - 1; ++i) {
        if (row[i] != 0 && row[i] == row[i + 1]) {
            row[i] *= 2;
            row[i + 1] = 0;
            score += row[i]; // Add to score
            merged = true;
            i++; // Skip next element since it's merged
        }
    }

    return merged;
}

// Main movement functions
bool Board::moveLeft() {
    bool moved = false;

    for (auto& row : grid) {
        bool compressed = compressRow(row);
        bool merged = mergeRow(row);
        bool compressedAgain = compressRow(row); // Compress again after merging

        if (compressed || merged || compressedAgain) {
            moved = true;
        }
    }

    if (moved) {
        addRandomTile();
    }

    return moved;
}

bool Board::moveRight() {
    // Reverse each row, move left, then reverse back
    bool moved = false;

    for (auto& row : grid) {
        std::reverse(row.begin(), row.end());
        bool compressed = compressRow(row);
        bool merged = mergeRow(row);
        bool compressedAgain = compressRow(row);
        std::reverse(row.begin(), row.end());

        if (compressed || merged || compressedAgain) {
            moved = true;
        }
    }

    if (moved) {
        addRandomTile();
    }

    return moved;
}

bool Board::moveUp() {
    bool moved = false;

    // Process columns instead of rows
    for (int col = 0; col < Constants::BOARD_SIZE; ++col) {
        // Extract column
        std::vector<int> column;
        for (int row = 0; row < Constants::BOARD_SIZE; ++row) {
            column.push_back(grid[row][col]);
        }

        // Move left logic on the column
        bool compressed = compressRow(column);
        bool merged = mergeRow(column);
        bool compressedAgain = compressRow(column);

        // Put column back
        for (int row = 0; row < Constants::BOARD_SIZE; ++row) {
            grid[row][col] = column[row];
        }

        if (compressed || merged || compressedAgain) {
            moved = true;
        }
    }

    if (moved) {
        addRandomTile();
    }

    return moved;
}

bool Board::moveDown() {
    bool moved = false;

    // Process columns instead of rows
    for (int col = 0; col < Constants::BOARD_SIZE; ++col) {
        // Extract column and reverse it
        std::vector<int> column;
        for (int row = 0; row < Constants::BOARD_SIZE; ++row) {
            column.push_back(grid[row][col]);
        }
        std::reverse(column.begin(), column.end());

        // Move left logic on the reversed column
        bool compressed = compressRow(column);
        bool merged = mergeRow(column);
        bool compressedAgain = compressRow(column);
        std::reverse(column.begin(), column.end());

        // Put column back
        for (int row = 0; row < Constants::BOARD_SIZE; ++row) {
            grid[row][col] = column[row];
        }

        if (compressed || merged || compressedAgain) {
            moved = true;
        }
    }

    if (moved) {
        addRandomTile();
    }

    return moved;
}