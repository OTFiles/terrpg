//File:main.cpp

#include "GameEngine.h"

int main() {
    GameEngine engine;
    engine.loadGame("game.txt");
    engine.startGameLoop();
    return 0;
}