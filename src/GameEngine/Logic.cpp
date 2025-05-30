// File: src/GameEngine/Logic.cpp
#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <algorithm>
#include "Log.h"

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

void GameEngine::processIfBlock(ifstream& fs, const string& rawCondition, int currentLine) {
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
        return any_of(inventory.begin(), inventory.end(), [&](const GameObject& item) {
            return item.name == parts[1] && item.getProperty("count", 1) > 0; });
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
    auto& currentMapObj = getCurrentMap();
    GameObject obj = currentMapObj.getObject(x, y);
    const bool stackable = obj.getProperty("stackable", 0);

#ifdef DEBUG
    Log debug_log("debug.log");
    debug_log.write("[DEBUG] Trying to pickup at (", x, ", ", y, ")");
    debug_log.write("[DEBUG] Object found: ", obj.name, " (type: ", obj.type, ")");
    debug_log.write("[DEBUG] Pickupable: ", obj.getProperty<int>("pickupable", 0));
#endif
    
    if(obj.type == "item" && obj.getProperty<int>("pickupable", 0) == 1) {
        // 堆叠逻辑
        if(stackable) {
            for(auto& existing : inventory) {
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
        inventory.push_back(newItem);
        currentMapObj.removeObject(x, y);
    }
}

void GameEngine::updateViewport() {
    viewportX = max(0, min(playerX - viewportW/2, 
        maps[currentMap].getWidth() - viewportW));
    viewportY = max(0, min(playerY - viewportH/2, 
        maps[currentMap].getHeight() - viewportH));
}

void GameEngine::useItem(const GameObject& item) {
    if (items.find(item.name) == items.end()) {
        showDialog("系统", "无效的物品: " + item.name);
        return; 
    }
    
    if (item.useEffects.empty()) {
        showDialog(item.name, "这个物品没有特殊效果");
        return;
    }
    
    for (const string& effect : items[item.name].useEffects) {
        vector<string> tokens = tokenize(effect);
        runCommand(tokens);
    }
    
    // 消耗品处理
    if (item.getProperty<int>("consumable", 0)) {
        auto it = find_if(inventory.begin(), inventory.end(), 
            [&](const GameObject& i) { return i.getProperty<int>("instance_id") == item.getProperty<int>("instance_id"); });
        
        if(it != inventory.end()) {
            int count = it->getProperty<int>("count", 1);
            if(count > 1) {
                it->setProperty("count", count - 1);
            } else {
                inventory.erase(it);
            }
            showDialog("系统", "已使用 " + item.name);
        }
    }
}

void GameEngine::discardItem(const GameObject& item) {
    auto it = find_if(inventory.begin(), inventory.end(),
        [&](const GameObject& i) { return i.getProperty<int>("instance_id") == item.getProperty<int>("instance_id"); });
    
    if(it != inventory.end()) {
        // 生成掉落物
        GameObject dropItem = *it;
        dropItem.x = playerX;
        dropItem.y = playerY;
        getCurrentMap().setObject(playerX, playerY, dropItem);
        
        // 减少数量或移除
        int count = it->getProperty<int>("count", 1);
        if(count > 1) {
            it->setProperty("count", count - 1);
        } else {
            inventory.erase(it);
        }
        showDialog("系统", "已丢弃 " + item.name);
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