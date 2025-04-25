// File: src/GameEngine/Logic.cpp
#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>

using namespace std;

// 辅助函数
vector<string> GameEngine::tokenize(const string& line) {
    istringstream iss(line);
    return {istream_iterator<string>{iss}, istream_iterator<string>{}};
}

char GameEngine::dirToChar(int dx, int dy) {
    if(dx == 1) return 'r';
    if(dx == -1) return 'l';
    if(dy == 1) return 'd';
    return 'u';
}

void GameEngine::parseLine(const string& line) {
    vector<string> tokens = tokenize(line);
    if(!tokens.empty()) runCommand(tokens);
}

void GameEngine::processIfBlock(ifstream& fs, 
                               const string& rawCondition,
                               int currentLine) {
    (void)currentLine;
    string condition = rawCondition;
    condition.erase(0, condition.find_first_not_of(" \t"));
    condition.erase(condition.find_last_not_of(" \t") + 1);
    
    if (condition.empty()) {
        throw runtime_error("Invalid empty condition after if");
    }

    bool execute = evalCondition(condition);
    int blockDepth = 1;
    string line;

    while (blockDepth > 0 && getline(fs, line)) {
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        if (line == "{") {
            blockDepth++;
        } else if (line == "}") {
            if (--blockDepth == 0) break;
        } else {
            if (execute && blockDepth == 1) {
                parseLine(line);
            }
        }
    }

    if (blockDepth != 0) {
        throw runtime_error("Unclosed condition block");
    }
}

bool GameEngine::evalCondition(const string& condition) {
    vector<string> parts = tokenize(condition);

    if (!parts.empty() && parts[0] == "have" && parts.size() >= 2) {
        return inventory.find(parts[1]) != inventory.end();
    }

    if (!parts.empty() && parts[0] == "去过" && parts.size() >= 2) {
        return visitedMarkers.find(parts[1]) != visitedMarkers.end();
    }

    if (parts.size() == 3 && parts[1] == "is") {
        int lhs = variables.count(parts[0]) ? variables[parts[0]] : 0;
        int rhs = evalExpression(parts[2]);
        return lhs == rhs;
    }

    string condNoSpace = condition;
    condNoSpace.erase(
        remove_if(condNoSpace.begin(), condNoSpace.end(), ::isspace),
        condNoSpace.end()
    );

    regex compRegex(R"(^(\w+)(==|!=|>=|<=|>|<)(.+)$)");
    smatch match;
    if (regex_match(condNoSpace, match, compRegex)) {
        string varName = match[1];
        string op = match[2];
        string rhsExpr = match[3];

        int lhs = variables.count(varName) ? variables[varName] : 0;
        int rhs = evalExpression(rhsExpr);

        if (op == "==") return lhs == rhs;
        if (op == "!=") return lhs != rhs;
        if (op == ">=") return lhs >= rhs;
        if (op == "<=") return lhs <= rhs;
        if (op == ">") return lhs > rhs;
        if (op == "<") return lhs < rhs;
    }

    throw runtime_error("无效条件格式: " + condition);
    return false;
}

string GameEngine::replaceVariables(const string& expr) {
    regex varRegex("\\{([^}]+)\\}");
    smatch match;
    string result = expr;
    
    while(regex_search(result, match, varRegex)) {
        string varName = match[1];
        int value = variables.count(varName) ? variables[varName] : 0;
        result.replace(match.position(), match.length(), to_string(value));
    }
    return result;
}

int GameEngine::evalExpression(const string& expr) {
    try {
        string processed = replaceVariables(expr);
        processed = regex_replace(processed, regex("\\s*([+\\-*/])\\s*"), "$1");
        if (processed.empty()) {
            throw invalid_argument("空表达式");
        }
        
        try {
            size_t pos;
            int result = stoi(processed, &pos);
            if(pos == processed.size()) return result;
        } catch(...) {}
        
        regex opRegex(R"((\d+)([+*/-])(\d+))");
        smatch match;
        if(regex_match(processed, match, opRegex)) {
            int a = stoi(match[1]);
            int b = stoi(match[3]);
            char op = match[2].str()[0];
            switch(op) {
                case '+': return a + b;
                case '-': return a - b;
                case '*': return a * b;
                case '/': return a / b;
            }
        }
        return 0;
    } catch (const exception& e) {
        showDialog("系统", "表达式解析错误: " + string(e.what()));
        return 0;
    }
}

void GameEngine::pickupItem(int x, int y) {
    GameObject obj = getCurrentMap().getObject(x, y);
    if(obj.type == "item" && obj.getProperty("可拾取", 0) == 1) {
        inventory.insert(obj.name);
        getCurrentMap().removeObject(x, y);
    }
}

void GameEngine::updateViewport() {
    viewportX = max(0, min(playerX - viewportW/2, 
        maps[currentMap].getWidth() - viewportW));
    viewportY = max(0, min(playerY - viewportH/2, 
        maps[currentMap].getHeight() - viewportH));
}

void GameEngine::useItem(const string& itemName) {
    if (items.find(itemName) == items.end()) {
        showDialog("系统", "无效的物品: " + itemName);
        return;
    }
    
    GameObject& item = items[itemName];
    if (item.useEffects.empty()) {
        showDialog(itemName, "这个物品没有特殊效果");
        return;
    }
    
    for (const string& effect : item.useEffects) {
        vector<string> tokens = tokenize(effect);
        runCommand(tokens);
    }
    
    if (item.getProperty("consumable", 0)) {
        inventory.erase(itemName);
        showDialog("系统", "已使用 " + itemName);
    }
}

void GameEngine::discardItem(const string& itemName) {
    if (inventory.erase(itemName)) {
        GameObject item = items[itemName];
        item.x = playerX;
        item.y = playerY;
        getCurrentMap().setObject(playerX, playerY, item);
        showDialog("系统", "已丢弃 " + itemName);
    }
}

void GameEngine::showDialog(const string& speaker, const string& content) {
    // 分割长文本为多行
    vector<string> lines;
    stringstream ss(content);
    string line;
    const size_t maxWidth = static_cast<size_t>(viewportW - 4);
    
    while (getline(ss, line, '\n')) {
        while (line.length() > maxWidth) {
            size_t spacePos = line.rfind(' ', maxWidth);
            if (spacePos == string::npos) spacePos = maxWidth;
            lines.push_back(line.substr(0, spacePos));
            line = line.substr(spacePos + 1);
        }
        if (!line.empty()) lines.push_back(line);
    }
    
    currentDialog = Dialog{lines, speaker};
    gameState = GameState::DIALOG;
}

void GameEngine::tryTalkToNPC() {
    // 检查四个方向是否有NPC
    const std::vector<std::pair<int, int>> directions = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}  // 上、下、左、右
    };
    
    for (const auto& [dx, dy] : directions) {
        GameObject obj = getCurrentMap().getObject(playerX + dx, playerY + dy);
        if (obj.type == "npc") {
            if (!obj.dialogues.empty()) {
                // 优先显示条件对话
                for (const auto& [cond, dialog] : obj.dialogues) {
                    if (cond != "default" && evalCondition(cond)) {
                        showDialog(obj.name, dialog);
                        return;
                    }
                }
                // 显示默认对话
                if (obj.dialogues.count("default")) {
                    showDialog(obj.name, obj.dialogues.at("default"));
                    return;
                }
            }
        }
    }
    showDialog("系统", "附近没有可对话的NPC");
}