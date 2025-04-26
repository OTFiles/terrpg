// File: src/GameMap.cpp
#include "GameMap.h"

GameMap::GameMap(int w, int h) : width(w), height(h) {}

void GameMap::setObject(int x, int y, const GameObject& obj) {
    objects[{x, y}] = obj;
}

GameObject GameMap::getObject(int x, int y) const {
    auto it = objects.find({x, y});
    return it != objects.end() ? it->second : GameObject();
}

void GameMap::removeObject(int x, int y) {
    objects.erase({x, y});
}

bool GameMap::hasObject(int x, int y) const {
    return objects.count({x, y}) > 0;
}

bool GameMap::hasObject(const std::string& name) const {
    for (const auto& [coord, obj] : objects) {
        if (obj.name == name) return true;
    }
    return false;
}

GameObject GameMap::getObjectByName(const std::string& name) const {
    for (const auto& [coord, obj] : objects) {
        if (obj.name == name) return obj;
    }
    return GameObject();
}

bool GameMap::isWalkable(int x, int y) const {
    if(x < 0 || x >= width || y < 0 || y >= height) return false;
    auto it = objects.find({x, y});
    if (it == objects.end()) return true;
    return it->second.getProperty<bool>("walkable", true);
}

void GameMap::fillArea(int x1, int y1, int x2, int y2, const GameObject& templateObj) {
    for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
        for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
            GameObject obj = templateObj;
            obj.x = x;
            obj.y = y;
            setObject(x, y, obj);
        }
    }
}