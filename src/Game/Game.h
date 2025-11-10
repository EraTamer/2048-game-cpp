#ifndef GAME_H
#define GAME_H

#include "Board.h"

class Game {
private:
    Board board;
    bool gameOver;
    bool won;

public:
    Game();
    void run();
    void processInput();
    void update();
    void render();
    bool isRunning() const;
};

#endif