// File: src/main.cpp
#include "GameEngine.h"

int main() {
    GameEngine engine;
    engine.loadGame("game.txt");
    engine.startGameLoop();
    engine.saveGame("data.txt");
    return 0;
}