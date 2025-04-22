// File: src/GameMap.cpp
#include "GameMap.h"

GameMap::GameMap(int w, int h) : width(w), height(h), grid(h, std::vector<GameObject>(w)) {}

void GameMap::setObject(int x, int y, const GameObject& obj) {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = obj;
    }
}

GameObject GameMap::getObject(int x, int y) const {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        return grid[y][x];
    }
    return GameObject();
}

void GameMap::removeObject(int x, int y) {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = GameObject();
    }
}

bool GameMap::isWalkable(int x, int y) const {
    if(x < 0 || x >= width || y < 0 || y >= height) return false;
    const GameObject& obj = grid[y][x];
    if(obj.type == "wall") return false;
    if(obj.type == "npc") return false;
    return true;
}