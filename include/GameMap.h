// File: GameMap.h
#pragma once
#include "GameObject.h"
#include <vector>

class GameMap {
private:
    int width;
    int height;
    std::vector<std::vector<GameObject>> grid;

public:
    explicit GameMap(int w = 20, int h = 20);
    void setObject(int x, int y, const GameObject& obj);
    GameObject getObject(int x, int y) const;
    void removeObject(int x, int y);
    bool isWalkable(int x, int y) const;
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};