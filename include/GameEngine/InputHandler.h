// include/InputHandler.h
#pragma once

class GameEngine;

class InputHandler {
    GameEngine& engine;
    
public:
    explicit InputHandler(GameEngine& eng) : engine(eng) {}
    
    void processInput(int key);

private:
    void handleExploring(int key);
    void handleInventory(int key);
    void handleItemOption(int key);
    void handleDialog(int key);
    void handleMovement(int dx, int dy);
};