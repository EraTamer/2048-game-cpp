#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

enum class MoveDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    INVALID,
    QUIT
};

class InputHandler {
public:
    static MoveDirection getMove();
};

#endif