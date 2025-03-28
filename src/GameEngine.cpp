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

void GameEngine::loadGame(const string& filename) {
    ifstream fs(filename);
    string line;
    vector<string> block;
    
    while(getline(fs, line)) {
        // 去除注释
        size_t commentPos = line.find('#');
        if(commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }
        if(line.empty()) continue;

        if(line.find("if") == 0) {
            processIfBlock(fs, line.substr(3));
        } else {
            parseLine(line);
        }
    }
}

void GameEngine::parseLine(const string& line) {
    vector<string> tokens = tokenize(line);
    if(!tokens.empty()) runCommand(tokens);
}

void GameEngine::processIfBlock(ifstream& fs, const string& condition) {
    vector<string> block;
    string line;
    bool execute = evalCondition(condition);
    
    while(getline(fs, line)) {
        if(line == "}") break;
        if(execute) parseLine(line);
    }
}

// 增强的条件判断
bool GameEngine::evalCondition(const string& condition) {
    // 分解条件类型
    vector<string> parts = tokenize(condition);
    
    if(parts[0] == "have") {
        return inventory.count(parts[1]) > 0;
    }
    else if(parts[0] == "去过") {
        return visitedMarkers.count(parts[1]) > 0;
    }
    
    // 变量条件判断
    regex compRegex(R"((\w+)(==|!=|>=|<=|>|<)(.+))");
    smatch match;
    if(regex_match(condition, match, compRegex)) {
        string varName = match[1];
        string op = match[2];
        string rhsExpr = match[3];
        
        int lhs = variables.count(varName) ? variables[varName] : 0;
        int rhs = evalExpression(rhsExpr);
        
        if(op == "==") return lhs == rhs;
        if(op == "!=") return lhs != rhs;
        if(op == ">=") return lhs >= rhs;
        if(op == "<=") return lhs <= rhs;
        if(op == ">") return lhs > rhs;
        if(op == "<") return lhs < rhs;
    }
    
    return false;
}

void GameEngine::runCommand(const vector<string>& tokens) {
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
                const string& mapName = tokens[2];    // 地图名
                int x = stoi(tokens[3]);              // x坐标
                int y = stoi(tokens[4]);              // y坐标
                const string& type = tokens[5];       // 类型 (wall/npc等)

                GameObject obj;
                obj.x = x;
                obj.y = y;
                obj.type = type;

                // 处理可选参数 [名称] 和 [显示字符]
                size_t nameIndex = 6;
                size_t displayIndex = 7;

                // 如果参数不足7个，可能省略了名称
                if (tokens.size() > nameIndex && tokens[nameIndex] != "") {
                    obj.name = tokens[nameIndex];
                }

                // 设置显示字符（优先使用参数，否则设置默认值）
                if (tokens.size() > displayIndex) {
                    obj.display = tokens[displayIndex][0];
                } else {
                    // 根据类型设置默认字符
                    if (type == "wall")       obj.display = '#';
                    if (type == "npc")        obj.display = '@';
                    if (type == "item")       obj.display = '$';
                    if (type == "陷阱")       obj.display = '^';
                    if (type == "标记点")     obj.display = '*';
                }

                // 保存到地图
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
    for(int y = 0; y < 20; ++y) {
        for(int x = 0; x < 20; ++x) {
            GameObject obj = curMap.getObject(x, y);
            if (obj.display != ' ') { // 只要显示字符不是空格就绘制
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
    string processed = replaceVariables(expr);
    
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
}

// 添加物品到背包
void GameEngine::pickupItem(int x, int y) {
    GameObject obj = getCurrentMap().getObject(x, y);
    if(obj.type == "item" && obj.getProperty("可拾取", 0)) {
        inventory.insert(obj.name);
        getCurrentMap().removeObject(x, y);
    }
}