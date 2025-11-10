#ifndef RENDERER_H
#define RENDERER_H

#include "../Game/Board.h"

class Renderer {
public:
    static void render(const Board& board);
    static void showGameOver();
    static void showWinMessage();
    static void clearScreen();
};

#endif