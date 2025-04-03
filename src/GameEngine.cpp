//文件:GameEngine.cpp
#include "GameEngine.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <ncurses.h>

using namespace std;

// 辅助函数
vector<string> tokenize(const string& line) {
    istringstream iss(line);
    return {istream_iterator<string>{iss}, istream_iterator<string>{}};
}

// 方向转换
char dirToChar(int dx, int dy) {
    if(dx == 1) return 'r';
    if(dx == -1) return 'l';
    if(dy == 1) return 'd';
    return 'u';
}

// GameObject成员函数
void GameObject::setProperty(const string& key, int value) {
    properties[key] = value;
}

int GameObject::getProperty(const string& key, int def) const {
    auto it = properties.find(key);
    return (it != properties.end()) ? it->second : def;
}

// GameEngine实现
GameEngine::GameEngine() : 
    currentMap("start"), 
    playerX(5), 
    playerY(5), 
    playerDir('d') {}

void GameEngine::processInitBlock(ifstream& fs, int& lineNumber) {
    int blockDepth = 1;
    string line;
    while (blockDepth > 0 && getline(fs, line)) {
        lineNumber++;
        // 处理注释和空白
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
            blockDepth--;
        } else {
            parseLine(line);
        }
    }
    if (blockDepth != 0) {
        throw runtime_error("未闭合的init块");
    }
}

void GameEngine::loadGame(const string& filename) {
    ifstream fs(filename);
    if (!fs.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }

    string line;
    int lineNumber = 0;
    while (getline(fs, line)) {
        lineNumber++;
        // 处理注释
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }
        // 清理空白
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        // 处理块结构
        if (line.find("init") == 0) { // 处理init块
            processInitBlock(fs, lineNumber);
        } else if (line.find("if ") == 0) {
            processIfBlock(fs, line.substr(3), lineNumber);
        } else {
            throw runtime_error("顶层命令必须位于init或if块中: " + line);
        }
    }

    // 确保存在main地图
    if (!maps.count("main")) {
        throw runtime_error("必须存在名为'main'的起始地图");
    }
    currentMap = "main";
}

void GameEngine::parseLine(const string& line) {
    vector<string> tokens = tokenize(line);
    if(!tokens.empty()) runCommand(tokens);
}

void GameEngine::processIfBlock(ifstream& fs, 
                                const string& rawCondition,
                                int currentLine) {
    (void)currentLine; // 显式标记参数未使用，在某次修改时删掉了，之后再处理吧
    string condition = rawCondition;
    // condition.erase(remove_if(condition.begin(), condition.end(), ::isspace), condition.end());
    condition.erase(0, condition.find_first_not_of(" \t"));
    condition.erase(condition.find_last_not_of(" \t") + 1);
    
    if (condition.empty()) {
        throw runtime_error("Invalid empty condition after if");
    }

    bool execute = evalCondition(condition);
    int blockDepth = 1;  // 支持嵌套块
    string line;

    while (blockDepth > 0 && getline(fs, line)) {
        // 统一注释处理
        size_t commentPos = line.find("//");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        // 清理行内容
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        // 块结构检测
        if (line == "{") {
            blockDepth++;
        } else if (line == "}") {
            if (--blockDepth == 0) break;
        } else {
            // 执行最外层命令
            if (execute && blockDepth == 1) {
                parseLine(line);
            }
        }
    }

    if (blockDepth != 0) {
        throw runtime_error("Unclosed condition block");
    }
}

// 条件判断
bool GameEngine::evalCondition(const string& condition) {
    vector<string> parts = tokenize(condition);

    // 处理 "have <物品>" 条件
    if (!parts.empty() && parts[0] == "have" && parts.size() >= 2) {
        return inventory.find(parts[1]) != inventory.end();
    }

    // 处理 "去过 <标记点>" 条件
    if (!parts.empty() && parts[0] == "去过" && parts.size() >= 2) {
        return visitedMarkers.find(parts[1]) != visitedMarkers.end();
    }

    // 处理 "变量 is 值" 条件
    if (parts.size() == 3 && parts[1] == "is") {
        int lhs = variables.count(parts[0]) ? variables[parts[0]] : 0;
        int rhs = evalExpression(parts[2]);
        return lhs == rhs;
    }

    // 处理比较运算符（==, !=, >=, <=, >, <）
    // 先移除所有空格以确保正则匹配正确
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

    // 无效条件格式
    throw runtime_error("无效条件格式: " + condition);
    return false;
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
                    } else {
                        throw runtime_error("缺少名称参数: " + type);
                    }
                }

                if (tokens.size() > displayIndex) {
                    string display = tokens[displayIndex];
                    obj.display = !display.empty() ? display[0] : ' ';
                } else {
                    // 设置默认显示字符
                    static map<string, char> defaults = {
                        {"wall", '#'}, {"npc", '@'}, {"item", '$'},
                        {"陷阱", '^'}, {"标记点", '*'}
                    };
                    obj.display = defaults.count(type) ? defaults[type] : '?';
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
                    item.setProperty("pickable", tokens[4] == "true");
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
    } catch(...) {
        // 简单的错误处理
    }
}

void GameEngine::showDialog(const string& npcName, const string& dialog) {
    clear();
    printw("【%s】: %s", npcName.c_str(), dialog.c_str());
    refresh();
    getch();
}

void GameEngine::saveGame(const string& filename) {
    ofstream fs(filename);
    // 保存地图状态
    for(auto& map : maps) {
        fs << "add map " << map.first << "\n";
        // 保存地图对象需要遍历所有格子...
    }
    // 保存NPC状态...
    // 保存物品状态...
}

void GameEngine::startGameLoop() {
    initscr();
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

void GameEngine::draw() {
    clear();
    GameMap& curMap = maps[currentMap];
    int width = curMap.getWidth();
    int height = curMap.getHeight();
    
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            GameObject obj = curMap.getObject(x, y);
            if (obj.display != ' ') {
                mvaddch(y, x, obj.display);
            }
        }
    }
    
    // 绘制玩家
    char playerChar = '>';
    switch(playerDir) {
        case 'u': playerChar = '^'; break;
        case 'd': playerChar = 'v'; break;
        case 'l': playerChar = '<'; break;
        case 'r': playerChar = '>'; break;
    }
    mvaddch(playerY, playerX, playerChar);
    refresh();
}

void GameEngine::handleInput(int key) {
    int dx = 0, dy = 0;
    switch(key) {
        case KEY_UP:    dy = -1; break;
        case KEY_DOWN:  dy = 1;  break;
        case KEY_LEFT:  dx = -1; break;
        case KEY_RIGHT: dx = 1;  break;
        case 'q': endwin(); exit(0);
    }
    
    if(dx != 0 || dy != 0) {
        playerDir = dirToChar(dx, dy);
        int newX = playerX + dx;
        int newY = playerY + dy;
        
        if(maps[currentMap].isWalkable(newX, newY)) {
            playerX = newX;
            playerY = newY;
        }
    }
    
    GameObject obj = getCurrentMap().getObject(playerX, playerY);
    if(obj.type == "标记点") {
        visitedMarkers.insert(obj.name);
    }
}

// GameMap实现
GameMap::GameMap(int w, int h) : width(w), height(h), grid(h, vector<GameObject>(w)) {}

void GameMap::setObject(int x, int y, const GameObject& obj) {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = obj;
    }
}

GameObject GameMap::getObject(int x, int y) const {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        return grid[y][x];
    }
    return GameObject();
}

void GameMap::removeObject(int x, int y) {
    if(x >= 0 && x < width && y >= 0 && y < height) {
        grid[y][x] = GameObject();
    }
}

bool GameMap::isWalkable(int x, int y) const {
    if(x < 0 || x >= width || y < 0 || y >= height) return false;
    const GameObject& obj = grid[y][x];
    if(obj.type == "wall") return false;
    if(obj.type == "npc") return false;
    return true;
}

// 辅助函数：替换字符串中的变量引用
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

// 辅助函数：计算表达式值
int GameEngine::evalExpression(const string& expr) {
    try {
        string processed = replaceVariables(expr);
        processed = regex_replace(processed, regex("\\s*([+\\-*/])\\s*"), "$1");
        if (processed.empty()) {
            throw invalid_argument("空表达式");
        }
        
        // 简单表达式解析（支持+-*/）
        try {
            size_t pos;
            int result = stoi(processed, &pos);
            if(pos == processed.size()) return result;
        } catch(...) {}
        
        // 处理运算符
        regex opRegex(R"((\d+)([+*/-])(\d+))");
        smatch match;
        if(regex_match(processed, match, opRegex)) {
            int a = stoi(match[1]);
            int b = stoi(match[2]);
            char op = match[3].str()[0];
            switch(op) {
                case '+': return a + b;
                case '-': return a - b;
                case '*': return a * b;
                case '/': return a / b;
            }
        }
        return 0; // 解析失败返回0
    } catch (const exception& e) {
        showDialog("系统", "表达式解析错误: " + string(e.what()));
        return 0;
    }
}

// 添加物品到背包
void GameEngine::pickupItem(int x, int y) {
    GameObject obj = getCurrentMap().getObject(x, y);
    if(obj.type == "item" && obj.getProperty("可拾取", 0)) {
        inventory.insert(obj.name);
        getCurrentMap().removeObject(x, y);
    }
}