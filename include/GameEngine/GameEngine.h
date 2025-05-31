// File: include/GameEngine.h
#pragma once
#include "GameState.h"
#include "GameMap.h"
#include "GameObject.h"
#include "ConditionEvaluator.h"
#include "DialogSystem.h"
#include "InventoryManager.h"
#include "SaveLoadManager.h"
#include "Renderer.h"
#include "InputHandler.h"
#include <map>
#include <set>
#include <vector>
#include <memory>

class GameEngine {
private:
    // 游戏核心数据
    std::map<std::string, GameMap> maps;
    std::map<std::string, GameObject> npcTemplates;
    std::map<std::string, GameObject> items;
    std::map<std::string, int> variables;
    std::set<std::string> visitedMarkers;

    // 子系统
    InventoryManager inventoryManager;
    DialogSystem dialogSystem;
    SaveLoadManager saveLoadManager;
    std::unique_ptr<Renderer> renderer;

    // 运行时状态
    std::string currentMap = "start";
    int playerX = 5;
    int playerY = 5;
    char playerDir = 'd';
    GameState gameState = GameState::EXPLORING;
    int viewportX = 0;
    int viewportY = 0;
    int viewportW = 20;
    int viewportH = 10;

public:
    GameEngine();
    
    // 核心接口
    void loadGame(const std::string& filename);
    void saveGame(const std::string& filename);
    void startGameLoop();
    void runCommand(const std::vector<std::string>& tokens);
    void showDialog(const std::string& speaker, const std::string& content);
    
    // 访问方法
    GameMap& getCurrentMap();
    const GameMap& getCurrentMap() const { return maps.at(currentMap); }
    InventoryManager& getInventoryManager() { return inventoryManager; }
    const InventoryManager& getInventoryManager() const { return inventoryManager; }
    DialogSystem& getDialogSystem() { return dialogSystem; }
    const DialogSystem& getDialogSystem() const { return dialogSystem; }
    std::set<std::string>& getVisitedMarkers() { return visitedMarkers; }
    const std::set<std::string>& getVisitedMarkers() const { return visitedMarkers; }
    
    // 游戏状态操作
    void updateViewport();
    void parseLine(const std::string& line);
    int generateItemInstanceId();
    
    std::vector<std::string> tokenize(const std::string& line);
    void pickupItem(int x, int y);
    void tryTalkToNPC();

    // 原始属性访问
    int& getPlayerX() { return playerX; }
    int& getPlayerY() { return playerY; }
    int getPlayerX() const { return playerX; }
    int getPlayerY() const { return playerY; }
    void setPlayerX(int x) { playerX = x; }
    void setPlayerY(int y) { playerY = y; }
    char& getPlayerDir() { return playerDir; }
    char getPlayerDir() const { return playerDir; }
    GameState& getGameState() { return gameState; }
    GameState getGameState() const { return gameState; }
    void setGameState(const GameState& state) { gameState = state; }
    int getViewportW() const { return viewportW; }
    int getViewportH() const { return viewportH; }
    bool evalCondition(const std::string& condition);
    
    std::map<std::string, GameMap>& getMaps() { return maps; }
    const std::map<std::string, GameMap>& getMaps() const { return maps; }
    std::map<std::string, GameObject>& getNpcs() { return npcTemplates; }
    std::map<std::string, GameObject>& getItems() { return items; }
    std::map<std::string, int>& getVariables() { return variables; }
    const std::map<std::string, int>& getVariables() const { return variables; }
    
    InputHandler inputHandler{*this};
    
    const std::list<GameObject>& getInventory() const { return inventoryManager.getItems(); }
        
    // 物品操作
    void useItem(const GameObject& item);
    void discardItem(const GameObject& item);
    void setCurrentMap(const std::string& map) { currentMap = map; }
    
    GameObject getObjectAt(int x, int y);

private:
    // 初始化方法
    void processInitBlock(std::ifstream& fs, int& lineNumber);
    void processIfBlock(std::ifstream& fs, const std::string& condition, int currentLine);
    void processItemEffectBlock(std::ifstream& fs, const std::string& headerLine, int& lineNumber);

    // 辅助方法
    static char dirToChar(int dx, int dy);
    
    friend class SaveLoadManager;
    friend class ConditionEvaluator;
};