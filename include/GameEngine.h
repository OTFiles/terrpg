// include/GameEngine.h
#pragma once
#include "GameState.h"
#include "GameMap.h"
#include "GameObject.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <optional>
#include <memory>
#include <fstream>

class GameEngine {
private:
    std::map<std::string, GameMap> maps;
    std::map<std::string, GameObject> npcs;
    std::map<std::string, GameObject> items;
    std::map<std::string, GameObject> npcTemplates;
    std::map<std::string, int> variables;
    std::set<std::string> visitedMarkers;
    std::set<std::string> inventory;

    std::string currentMap = "start";
    int playerX = 5;
    int playerY = 5;
    char playerDir = 'd';
    
    GameState gameState = GameState::EXPLORING;
    int selectedInventoryIndex = 0;
    std::optional<Dialog> currentDialog;
    int viewportX = 0;
    int viewportY = 0;
    const int viewportW = 20;
    const int viewportH = 10;

    // Private method declarations
    void processInitBlock(std::ifstream& fs, int& lineNumber);
    void processItemEffectBlock(std::ifstream& fs, const std::string& headerLine, int& lineNumber);
    void updateViewport();
    void tryTalkToNPC();
    void parseLine(const std::string& line);
    void processIfBlock(std::ifstream& fs, const std::string& condition, int currentLine);
    static std::vector<std::string> tokenize(const std::string& line);
    void drawUI(); 
    char dirToChar(int dx, int dy);
    std::string replaceVariables(const std::string& expr);
    void handleExploringInput(int key);
    void handleInventoryInput(int key);
    void handleItemOptionInput(int key);
    void handleDialogInput(int key);

public:
    GameEngine() : currentMap("start"), playerX(5), playerY(5), playerDir('d') {}
    void loadGame(const std::string& filename);
    void saveGame(const std::string& filename);
    void startGameLoop();
    void showDialog(const std::string& speaker, const std::string& dialog);
    GameMap& getCurrentMap() { return maps[currentMap]; }
    
    // Additional forward declarations for methods implemented in other cpp files
    void handleInput(int key);
    void draw();
    void runCommand(const std::vector<std::string>& tokens);
    bool evalCondition(const std::string& condition);
    int evalExpression(const std::string& expr);
    void pickupItem(int x, int y);
    std::ifstream* getCurrentFileStream();
    int& getCurrentLineNumber();
    void useItem(const std::string& itemName);
    void discardItem(const std::string& itemName);
    std::map<std::string, GameMap>& getMaps() { return maps; }
    std::map<std::string, GameObject>& getNpcs() { return npcs; }
    std::map<std::string, GameObject>& getItems() { return items; }
    std::map<std::string, GameObject>& getNpcTemplates() { return npcTemplates; }
    std::map<std::string, int>& getVariables() { return variables; }
    void setCurrentMap(const std::string& map) { currentMap = map; }
    void setPlayerX(int x) { playerX = x; }
    void setPlayerY(int y) { playerY = y; }
};