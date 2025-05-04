// File: src/GameEngine/LoadSave.cpp
#include "GameEngine.h"
#include <fstream>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <ncurses.h>

static std::ifstream* currentFileStream = nullptr;
static int currentLineNumber = 0;

std::ifstream* GameEngine::getCurrentFileStream() { return currentFileStream; }
int& GameEngine::getCurrentLineNumber() { return currentLineNumber; }

void GameEngine::processInitBlock(std::ifstream& fs, int& lineNumber) {
    int blockDepth = 1;
    std::string line;
    while (blockDepth > 0 && getline(fs, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        if (line == "{") blockDepth++;
        else if (line == "}") blockDepth--;
        else parseLine(line);
    }
#ifdef DEBUG
    std::clog << "[DEBUG] Init block processed. Total items defined: " << items.size() << std::endl;
#endif
    if (blockDepth != 0) throw std::runtime_error("Unclosed init block");
}

void GameEngine::processItemEffectBlock(std::ifstream& fs, const std::string& headerLine, int& lineNumber) {
    std::vector<std::string> tokens = tokenize(headerLine);
    if(tokens.size() < 3 || tokens[2].back() != ':')
        throw std::runtime_error("Invalid item effect format: " + headerLine);
    
    std::string itemName = tokens[2].substr(0, tokens[2].size()-1);
    if(items.find(itemName) == items.end())
        throw std::runtime_error("Undefined item: " + itemName);

    int blockDepth = 1;
    std::string line;
    while (blockDepth > 0 && getline(fs, line)) {
        lineNumber++;
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if(line == "{") blockDepth++;
        else if(line == "}") blockDepth--;
        else items[itemName].useEffects.push_back(line);
    }
}

void GameEngine::loadGame(const std::string& filename) {
    static std::ifstream fs;
    fs.open(filename);
    currentFileStream = &fs;
    
    if (!fs.is_open()) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (getline(fs, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        if (line.find("init") == 0) processInitBlock(fs, lineNumber);
        else if (line.find("if ") == 0) processIfBlock(fs, line.substr(3), lineNumber);
        else if (line.find("item 使用效果") == 0) processItemEffectBlock(fs, line, lineNumber);
        else throw std::runtime_error("Top-level commands must be in init/if/item blocks: " + line);
    }

    if (!maps.count("main")) throw std::runtime_error("Missing 'main' start map");
    currentMap = "main";
    
#ifdef DEBUG
    std::clog << "[DEBUG] Loaded game with " << inventory.size() << " initial items\n";
    std::clog << "[DEBUG] Player starting position: (" << playerX << ", " << playerY << ")\n";
#endif
}

void GameEngine::startGameLoop() {
    initscr();
    setlocale(LC_ALL, "");  // 启用本地化支持
    use_default_colors();    // 使用终端默认颜色
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    while(true) {
        draw();
        int ch = getch();
        handleInput(ch);
    }
    endwin();
}

void GameEngine::saveGame(const std::string& filename) {
    (void)filename; //可恶，因为还没做所以多了一个报错QWQ
    // TODO
}

void GameEngine::handleInput(int key) {
    switch (static_cast<int>(gameState)) {
        case static_cast<int>(GameState::EXPLORING):
            handleExploringInput(key);
            break;
        case static_cast<int>(GameState::INVENTORY):
            handleInventoryInput(key);
            break;
        case static_cast<int>(GameState::ITEM_OPTION):
            handleItemOptionInput(key);
            break;
        case static_cast<int>(GameState::DIALOG):
            handleDialogInput(key);
            break;
    }
}

void GameEngine::handleExploringInput(int key) {
    int dx = 0, dy = 0;
    
    // 移动处理
    switch(key) {
        case KEY_UP:    dy = -1; playerDir = 'u'; break;
        case KEY_DOWN:  dy = 1;  playerDir = 'd'; break;
        case KEY_LEFT:  dx = -1; playerDir = 'l'; break;
        case KEY_RIGHT: dx = 1;  playerDir = 'r'; break;
        case 'g': // 拾取物品
            pickupItem(playerX, playerY);
            break;
        case 'u': // 与NPC对话
            tryTalkToNPC();
            break;
        case 'i': // 打开物品栏
            if (!inventory.empty()) {
                gameState = GameState::INVENTORY;
                selectedInventoryIndex = 0;
            } else {
                showDialog("系统", "物品栏为空");
            }
            break;
        case 'q': // 退出游戏
            endwin();
            exit(0);
    }
    
    // 移动玩家
    if (dx != 0 || dy != 0) {
        int newX = playerX + dx;
        int newY = playerY + dy;
        
        if (maps[currentMap].isWalkable(newX, newY)) {
            playerX = newX;
            playerY = newY;
            updateViewport();
        }
    }
}

void GameEngine::handleInventoryInput(int key) {
    switch(key) {
        case KEY_UP:
            if (selectedInventoryIndex > 0) selectedInventoryIndex--;
            break;
        case KEY_DOWN:
            if (selectedInventoryIndex < (int)inventory.size()-1) selectedInventoryIndex++;
            break;
        case '\n': { // 回车选择物品
            auto it = inventory.begin();
            std::advance(it, selectedInventoryIndex);
            currentDialog = Dialog{
                {"使用", "丢弃"}, 
                *it
            };
            gameState = GameState::ITEM_OPTION;
            break;
        }
        case 27: // ESC键返回
            gameState = GameState::EXPLORING;
            break;
    }
}

void GameEngine::handleItemOptionInput(int key) {
    switch(key) {
        case KEY_UP:
        case KEY_DOWN:
            // 切换选项
            if (!currentDialog->lines.empty()) {
                std::rotate(currentDialog->lines.begin(), 
                          currentDialog->lines.begin() + 1, 
                          currentDialog->lines.end());
            }
            break;
        case '\n': {
            auto it = inventory.begin();
            std::advance(it, selectedInventoryIndex);
            if (currentDialog->lines[0] == "使用") {
                useItem(*it);
            } else {
                discardItem(*it);
            }
            gameState = GameState::EXPLORING;
            break;
        }
        case 27: // ESC键返回
            gameState = GameState::INVENTORY;
            break;
    }
}

void GameEngine::handleDialogInput(int key) {
    if (key != ERR) {
        currentDialog.reset();
        gameState = GameState::EXPLORING;
    }
}