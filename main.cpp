#define _GLIBCXX_USE_CXX11_ABI 0
#include "Game.h"
#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cerr << "GL initialization failed" << std::endl;
        return -1;
    }

    Game g = Game();
    return g.start();
}