// File: GameEngine.h
#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <regex>

struct GameObject {
    int x = 0;
    int y = 0;
    char display = ' ';
    std::string name;
    std::string type;
    std::map<std::string, int> properties;
    std::map<std::string, std::string> dialogues;

    void setProperty(const std::string& key, int value) {
        properties[key] = value;
    }

    int getProperty(const std::string& key, int def = 0) const {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : def;
    }
};

class GameMap {
private:
    int width;
    int height;
    std::vector<std::vector<GameObject>> grid;

public:
    GameMap(int w = 20, int h = 20) : width(w), height(h), grid(h, std::vector<GameObject>(w)) {}

    void setObject(int x, int y, const GameObject& obj) {
        if(x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x] = obj;
        }
    }

    GameObject getObject(int x, int y) const {
        if(x >= 0 && x < width && y >= 0 && y < height) {
            return grid[y][x];
        }
        return GameObject();
    }

    void removeObject(int x, int y) {
        if(x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x] = GameObject();
        }
    }

    bool isWalkable(int x, int y) const {
        if(x < 0 || x >= width || y < 0 || y >= height) return false;
        const GameObject& obj = grid[y][x];
        if(obj.type == "wall") return false;
        if(obj.type == "npc") return false;
        return true;
    }
};

class GameEngine {
private:
    // 游戏数据存储
    std::map<std::string, GameMap> maps;
    std::map<std::string, GameObject> npcs;
    std::map<std::string, GameObject> items;
    std::map<std::string, int> variables;
    std::set<std::string> visitedMarkers;
    std::set<std::string> inventory;

    // 玩家状态
    std::string currentMap = "start";
    int playerX = 5;
    int playerY = 5;
    char playerDir = 'd'; // u/d/l/r

    // 私有方法
    void parseLine(const std::string& line);
    void processIfBlock(std::ifstream& fs, const std::string& condition);
    bool evalCondition(const std::string& condition);
    int evalExpression(const std::string& expr);
    std::string replaceVariables(const std::string& expr);
    void handleCollision(int x, int y);

public:
    GameEngine() = default;
    
    // 核心功能
    void loadGame(const std::string& filename);
    void saveGame(const std::string& filename);
    void runCommand(const std::vector<std::string>& tokens);
    
    // 游戏循环
    void startGameLoop();
    void draw();
    void handleInput(int key);
    void showDialog(const std::string& speaker, const std::string& dialog);
    
    // 辅助功能
    void pickupItem(int x, int y);
    GameMap& getCurrentMap() { return maps[currentMap]; }
};