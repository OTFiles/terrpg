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

void GameEngine::runCommand(const vector<string>& tokens) {
    if (tokens.empty()) return;
    try {
        if(tokens[0] == "add") {
            if(tokens[1] == "map") {
                maps[tokens[2]] = GameMap(20, 20);
            }
            else if(tokens[1] == "npc") {
                const string& name = tokens[2];
                GameObject npc;
                npc.type = "npc";
                npc.name = name;
                npcs[name] = npc;
            }
            else if(tokens[1] == "item") {
                GameObject item;
                item.type = "item";
                item.name = tokens[2];
                items[tokens[2]] = item;
            }
            else if(tokens[1] == "变量") {
                variables[tokens[2]] = 0;
            }
        }
        else if(tokens[0] == "set") {
            if(tokens[1] == "map") {
                const string& mapName = tokens[2];
                int x = stoi(tokens[3]);
                int y = stoi(tokens[4]);
                const string& type = tokens[5];

                GameObject obj;
                obj.x = x;
                obj.y = y;
                obj.type = type;

                bool requiresName = (type == "npc" || type == "item" || type == "陷阱" || type == "标记点");
                size_t nameIndex = 6;
                size_t displayIndex = 6;

                if (requiresName) {
                    if (tokens.size() > nameIndex) {
                        obj.name = tokens[nameIndex];
                        displayIndex = nameIndex + 1;

                        // 特殊处理物品类型 - 从模板复制属性
                        if (type == "item") {
                            if (items.count(obj.name)) {
                                GameObject& templateItem = items[obj.name];
                                obj.properties = templateItem.properties;
                                obj.useEffects = templateItem.useEffects;
                                obj.display = templateItem.display; // 优先使用模板显示字符
                            }
                        }
                    } else {
                        throw runtime_error("缺少名称参数: " + type);
                    }
                }

                // 处理显示字符（只有当模板未设置时才覆盖）
                if (tokens.size() > displayIndex && obj.display == ' ') {
                    string display = tokens[displayIndex];
                    obj.display = !display.empty() ? display[0] : ' ';
                }

                // 设置默认显示字符（如果仍未设置）
                if (obj.display == ' ') {
                    static map<string, char> defaults = {
                        {"wall", '#'}, {"npc", '@'}, {"item", '$'},
                        {"陷阱", '^'}, {"标记点", '*'}
                    };
                    obj.display = defaults.count(type) ? defaults[type] : '?';
                }

                // 设置默认属性
                if (type == "wall") {
                    obj.setProperty("walkable", 0);
                } else if (type == "陷阱") {
                    obj.setProperty("damage", 10);
                    obj.setProperty("walkable", 1);
                }

                maps[mapName].setObject(x, y, obj);
            }
            else if(tokens[1] == "npc") {
                const string& name = tokens[2];
                if(tokens[3] == "对话") {
                    string condition = tokens.size() > 4 ? tokens[4] : "always";
                    string dialog = tokens.back();
                    npcs[name].dialogues[condition] = dialog;
                }
            }
            else if(tokens[1] == "item") {
                GameObject& item = items[tokens[2]];
                if(tokens[3] == "伤害") {
                    item.setProperty("damage", stoi(tokens[4]));
                }
                else if(tokens[3] == "可拾取") {
                    item.setProperty("可拾取", tokens[4] == "true" ? 1 : 0);
                }
            }
            else if(tokens[1] == "变量") {
                string varName = tokens[2];
                string expr;
                for(size_t i=3; i<tokens.size(); ++i) {
                    expr += tokens[i];
                }
                variables[varName] = evalExpression(expr);
            }
        }
        else if(tokens[0] == "tp") {
            currentMap = tokens[1];
            playerX = stoi(tokens[2]);
            playerY = stoi(tokens[3]);
        }
        else if(tokens[0] == "run") {
            if(tokens[1] == "npc") {
                showDialog(tokens[2], tokens[4]);
            }
        }
        else if (tokens[0] == "fill") {
            // 参数格式: fill <地图名> <x1> <y1> <x2> <y2> (wall|陷阱) [陷阱名称]
            if (tokens.size() < 7) {
                showDialog("系统", "错误：fill命令参数不足！");
                return;
            }

            std::string mapName = tokens[1];
            std::string type = tokens[6];
            bool isTrap = (type == "陷阱");
            size_t requiredParams = isTrap ? 8 : 7;

            if (tokens.size() != requiredParams) {
                showDialog("系统", "错误：fill命令参数数量不正确！");
                return;
            }

            // 检查地图是否存在
            if (maps.find(mapName) == maps.end()) {
                showDialog("系统", "错误：地图" + mapName + "不存在！");
                return;
            }

            // 解析坐标
            int x1, y1, x2, y2;
            try {
                x1 = std::stoi(tokens[2]);
                y1 = std::stoi(tokens[3]);
                x2 = std::stoi(tokens[4]);
                y2 = std::stoi(tokens[5]);
            } catch (...) {
                showDialog("系统", "错误：坐标必须是整数！");
                return;
            }

            GameMap& targetMap = maps[mapName];
            int mapWidth = targetMap.getWidth();
            int mapHeight = targetMap.getHeight();

            // 确定填充区域
            int minX = std::min(x1, x2);
            int maxX = std::max(x1, x2);
            int minY = std::min(y1, y2);
            int maxY = std::max(y1, y2);

            // 边界检查
            if (minX < 0 || maxX >= mapWidth || minY < 0 || maxY >= mapHeight) {
                showDialog("系统", "错误：坐标超出地图范围！");
                return;
            }

            // 创建对象模板
            std::string trapName;
            GameObject obj;
            if (type == "wall") {
                obj.type = "wall";
                obj.display = '#';
                obj.setProperty("walkable", 0);
                obj.name = "wall";
            } else if (isTrap) {
                trapName = tokens[7];
                obj.type = "trap";
                obj.display = '^';
                obj.name = trapName;
                obj.setProperty("damage", 10);
                obj.setProperty("walkable", 1);
            }

            // 填充区域
            for (int x = minX; x <= maxX; ++x) {
                for (int y = minY; y <= maxY; ++y) {
                    targetMap.setObject(x, y, obj);
                }
            }

            showDialog("系统", "填充操作已完成！");
        }
    } catch(const exception& e) {
        showDialog("错误", string("命令执行失败: ") + e.what());
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