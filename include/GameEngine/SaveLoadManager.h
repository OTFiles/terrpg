// include/SaveLoadManager.h
#pragma once
#include "GameObject.h"
#include <fstream>
#include <sstream>
#include <unordered_set>

class GameEngine;

class SaveLoadManager {
public:
    void saveState(const GameEngine& engine, const std::string& filename);
    void loadState(GameEngine& engine, const std::string& filename);

private:
    void serializeGameObject(std::ostream& os, const GameObject& obj);
    GameObject deserializeGameObject(std::istream& is);
    std::string escapeString(const std::string& str);
    std::string unescapeString(const std::string& str);
};