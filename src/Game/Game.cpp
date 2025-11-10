#include "Game.h"
#include "../Render/Renderer.h"
#include "../Input/InputHandler.h"
#include <iostream>

using namespace std;

Game::Game() : gameOver(false), won(false) {
    board.initialize();
}

void Game::run() {
    while (isRunning()) {
        render();
        processInput();
        update();
    }
}

void Game::processInput() {
    MoveDirection direction = InputHandler::getMove();

    switch(direction) {
        case MoveDirection::UP:
            board.moveUp();
            break;
        case MoveDirection::DOWN:
            board.moveDown();
            break;
        case MoveDirection::LEFT:
            board.moveLeft();
            break;
        case MoveDirection::RIGHT:
            board.moveRight();
            break;
        case MoveDirection::QUIT:
            gameOver = true;
            break;
        case MoveDirection::INVALID:
            // Do nothing for invalid input
            break;
    }
}

void Game::update() {
    // Check win condition
    if (board.hasWon() && !won) {
        won = true;
        cout << "Congratulations! You reached 2048!" << endl;
    }

    // Check game over condition
    if (board.isGameOver() && !gameOver) {
        gameOver = true;
        cout << "Game Over! No more moves possible." << endl;
    }
}

void Game::render() {
    Renderer::render(board);
}

bool Game::isRunning() const {
    return !gameOver && !won;
}