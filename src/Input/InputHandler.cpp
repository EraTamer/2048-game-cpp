#include "InputHandler.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace std;

// Helper function to set terminal to raw mode
void setTerminalRaw(bool enable) {
    static struct termios oldt, newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

// Helper function to read a single character without Enter
char getch() {
    char buf = 0;
    setTerminalRaw(true);
    if (read(STDIN_FILENO, &buf, 1) < 0) {
        setTerminalRaw(false);
        return 0;
    }
    setTerminalRaw(false);
    return buf;
}

MoveDirection InputHandler::getMove() {
    char input = getch();

    // Handle arrow keys (ANSI escape sequences)
    if (input == '\033') { // Escape character
        getch(); // Skip '['
        switch(getch()) {
            case 'A': return MoveDirection::UP;
            case 'B': return MoveDirection::DOWN;
            case 'C': return MoveDirection::RIGHT;
            case 'D': return MoveDirection::LEFT;
        }
    }

    // Handle regular keys
    switch(input) {
        case 'w': case 'W': return MoveDirection::UP;
        case 's': case 'S': return MoveDirection::DOWN;
        case 'a': case 'A': return MoveDirection::LEFT;
        case 'd': case 'D': return MoveDirection::RIGHT;
        case 'q': case 'Q': return MoveDirection::QUIT;
        case 3: // Ctrl+C
            return MoveDirection::QUIT;
    }

    return MoveDirection::INVALID;
}