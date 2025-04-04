// File: GameEngine.h
#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <regex>
#include <optional>
#include <memory>

enum class GameState { 
    EXPLORING, 
    INVENTORY, 
    ITEM_OPTION, 
    DIALOG
};

struct Dialog {
    std::vector<std::string> lines;
    std::string speaker;
};

struct GameObject {
    int x = 0;
    int y = 0;
    char display = ' ';
    std::string name;
    std::string type;
    std::map<std::string, int> properties;
    std::map<std::string, std::string> dialogues;

    void setProperty(const std::string& key, int value);
    int getProperty(const std::string& key, int def = 0) const;
    
    std::vector<std::string> useEffects;
};

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

class __attribute__((visibility("default"))) GameEngine {
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
    void processIfBlock(std::ifstream& fs, 
                        const std::string& condition,
                        int currentLine);
    bool evalCondition(const std::string& condition);
    int evalExpression(const std::string& expr);
    std::string replaceVariables(const std::string& expr);
    void handleCollision(int x, int y);
    void processInitBlock(std::ifstream& fs, int& lineNumber);
    
    GameState gameState = GameState::EXPLORING;
    int selectedInventoryIndex = 0;
    std::optional<Dialog> currentDialog;
    int viewportX = 0;
    int viewportY = 0;
    const int viewportW = 20;
    const int viewportH = 10;
    
    void checkNPCCollision();
    void updateViewport();
    void drawUI();
    void drawInventory();
    void useItem(const std::string& item);
    void discardItem(const std::string& item);
    
    void handleExploringInput(int key);
    void handleInventoryInput(int key);
    void handleItemOptionInput(int key);
    void handleDialogInput(int key);
    void tryTalkToNPC();
    
    std::ifstream* getCurrentFileStream();
    int& getCurrentLineNumber();
    void processItemEffectBlock(std::ifstream& fs, const std::string& headerLine, int& lineNumber);

public:
    GameEngine();
    
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