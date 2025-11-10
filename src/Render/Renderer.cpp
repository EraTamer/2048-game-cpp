#include "Renderer.h"
#include <iostream>
#include <iomanip>

using namespace std;

void Renderer::render(const Board& board) {
    clearScreen();

    cout << "=== 2048 GAME ===" << endl;
    cout << "Score: " << board.getScore() << endl << endl;

    const auto& grid = board.getGrid();

    // Draw the top border
    cout << "+------+------+------+------+" << endl;

    for (int i = 0; i < Constants::BOARD_SIZE; ++i) {
        cout << "|";
        for (int j = 0; j < Constants::BOARD_SIZE; ++j) {
            if (grid[i][j] == 0) {
                cout << "      |";
            } else {
                cout << " " << setw(4) << grid[i][j] << " |";
            }
        }
        cout << endl;

        // Draw inner borders or bottom border
        if (i < Constants::BOARD_SIZE - 1) {
            cout << "+------+------+------+------+" << endl;
        } else {
            cout << "+------+------+------+------+" << endl;
        }
    }

    cout << endl;
    cout << "Controls: Arrow Keys to move, 'q' to quit" << endl;

    // Check game state
    if (board.hasWon()) {
        showWinMessage();
    } else if (board.isGameOver()) {
        showGameOver();
    }
}

void Renderer::showGameOver() {
    cout << "=================================" << endl;
    cout << "          GAME OVER!" << endl;
    cout << "=================================" << endl;
}

void Renderer::showWinMessage() {
    cout << "=================================" << endl;
    cout << "          YOU WIN!" << endl;
    cout << "=================================" << endl;
}

void Renderer::clearScreen() {
    // ANSI escape codes to clear screen and move cursor to top-left
    cout << "\033[2J\033[1;1H";
}