// File: src/GameEngine/GameEngine.cpp
#include "GameEngine.h"
#include "Commands/CommandParser.h"
#include "Log.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <ncurses.h>

GameEngine::GameEngine() : renderer(std::make_unique<Renderer>()), inputHandler(*this) {}

// 核心游戏循环
void GameEngine::startGameLoop() {
    renderer->initScreen();
    while(true) {
        renderer->render(*this);
        int ch = getch();
        inputHandler.processInput(ch);
    }
}

// 命令执行入口
void GameEngine::runCommand(const std::vector<std::string>& tokens) {
    std::string commandLine;
    for (const auto& token : tokens) {
        if (!commandLine.empty()) commandLine += " ";
        commandLine += token;
    }
    CommandParser::parseAndExecute(commandLine, *this);
}

// 地图相关方法
GameMap& GameEngine::getCurrentMap() { 
    return maps[currentMap];
}

GameObject GameEngine::getObjectAt(int x, int y) {
    return getCurrentMap().getObject(x, y);
}

// 视口计算
void GameEngine::updateViewport() {
    const GameMap& currentMapObj = getCurrentMap();
    viewportX = std::max(0, std::min(playerX - viewportW/2, 
        currentMapObj.getWidth() - viewportW));
    viewportY = std::max(0, std::min(playerY - viewportH/2, 
        currentMapObj.getHeight() - viewportH));
}

// 脚本解析核心
void GameEngine::parseLine(const std::string& line) {
    std::vector<std::string> tokens = tokenize(line);
    if(!tokens.empty()) runCommand(tokens);
}

// 初始化块处理
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
    if (blockDepth != 0) throw std::runtime_error("Unclosed init block");
}

// 条件块处理
void GameEngine::processIfBlock(std::ifstream& fs, const std::string& rawCondition, int currentLine) {
    (void)currentLine;
    std::string condition = rawCondition;
    condition.erase(0, condition.find_first_not_of(" \t"));
    condition.erase(condition.find_last_not_of(" \t") + 1);
    
    bool execute = ConditionEvaluator::evaluate(*this, condition);
    int blockDepth = 1;
    std::string line;

    while (blockDepth > 0 && getline(fs, line)) {
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        if (line == "{") blockDepth++;
        else if (line == "}") blockDepth--;
        else if (execute && blockDepth == 1) parseLine(line);
    }
}

// 物品效果块处理
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

// 辅助方法
std::vector<std::string> GameEngine::tokenize(const std::string& line) {
    std::istringstream iss(line);
    return {std::istream_iterator<std::string>{iss}, 
            std::istream_iterator<std::string>{}};
}

char GameEngine::dirToChar(int dx, int dy) {
    if(dx == 1) return 'r';
    if(dx == -1) return 'l';
    if(dy == 1) return 'd';
    return 'u';
}

int GameEngine::generateItemInstanceId() {
    static int counter = 0;
    return ++counter;
}

bool GameEngine::evalCondition(const std::string& condition) {
    return ConditionEvaluator::evaluate(*this, condition);
}

void GameEngine::tryTalkToNPC() {
    dialogSystem.tryTalkToNPC(*this);
}

void GameEngine::pickupItem(int x, int y) {
    auto& currentMapObj = getCurrentMap();
    GameObject obj = currentMapObj.getObject(x, y);
    const bool stackable = obj.getProperty("stackable", 0);

    if(obj.type == "item" && obj.getProperty<int>("pickupable", 0) == 1) {
        // 堆叠逻辑
        if(stackable) {
            for(auto& existing : inventoryManager.getItems()) {
                if(existing.name == obj.name) {
                    int curCount = existing.getProperty<int>("count", 1);
                    existing.setProperty("count", curCount + 1);
                    currentMapObj.removeObject(x, y);
                    return;
                }
            }
        }
        
        // 创建新实例
        GameObject newItem = obj;
        newItem.setProperty("instance_id", generateItemInstanceId());
        newItem.setProperty("count", 1);
        inventoryManager.addItem(newItem);
        currentMapObj.removeObject(x, y);
    }
}

void GameEngine::useItem(const GameObject& item) {
    if (items.find(item.name) == items.end()) {
        dialogSystem.showDialog({{"无效的物品: " + item.name}, "系统"}, *this);
        return; 
    }
    
    for (const std::string& effect : items[item.name].useEffects) {
        std::vector<std::string> tokens = tokenize(effect);
        runCommand(tokens);
    }
    
    // 消耗品处理
    if (item.getProperty<int>("consumable", 0)) {
        inventoryManager.removeItem(item);
    }
}

void GameEngine::discardItem(const GameObject& item) {
    auto& currentMap = getCurrentMap();
    GameObject dropItem = item;
    
    if (item.getProperty<int>("count", 1) > 1) {
        dropItem.setProperty("count", 1);
        auto& stack = inventoryManager.getItems();
        auto it = find_if(stack.begin(), stack.end(), 
            [&](const GameObject& i) { return i.getProperty<int>("instance_id") == item.getProperty<int>("instance_id"); });
        it->setProperty("count", it->getProperty<int>("count", 1) - 1);
    } else {
        inventoryManager.removeItem(item);
    }
    
    currentMap.setObject(playerX, playerY, dropItem);
}

// 文件加载入口
void GameEngine::loadGame(const std::string& filename) {
    std::ifstream fs(filename);
    if (!fs.is_open()) throw std::runtime_error("无法打开游戏文件: " + filename);

    int lineNumber = 0;
    std::string line;
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
        else throw std::runtime_error("顶层命令必须在init/if/item块内: " + line);
    }

    if (!maps.count("main")) throw std::runtime_error("缺少主地图'main'");
    currentMap = "main";
    
#ifdef DEBUG
    dialogSystem.showDialog({{"你好，旅行者！", "这是我的第二行对话内容"}, "测试对话功能"}, *this);
#endif
}

// 游戏保存入口
void GameEngine::saveGame(const std::string& filename) {
    saveLoadManager.saveState(*this, filename);
}