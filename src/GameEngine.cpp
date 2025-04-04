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

static ifstream* currentFileStream = nullptr;
static int currentLineNumber = 0;

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

void GameEngine::processItemEffectBlock(ifstream& fs, const string& headerLine, int& lineNumber) {
    // 解析物品名称
    vector<string> tokens = tokenize(headerLine);
    if(tokens.size() < 3 || tokens[2].back() != ':') {
        throw runtime_error("无效的item效果格式: " + headerLine);
    }
    
    string itemName = tokens[2].substr(0, tokens[2].size()-1);
    if(items.find(itemName) == items.end()) {
        throw runtime_error("未定义的物品: " + itemName);
    }

    // 处理效果块
    int blockDepth = 1;
    string line;
    while (blockDepth > 0 && getline(fs, line)) {
        lineNumber++;
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if(line == "{") {
            blockDepth++;
        } else if(line == "}") {
            if(--blockDepth == 0) break;
        } else {
            items[itemName].useEffects.push_back(line);
        }
    }
}

void GameEngine::loadGame(const string& filename) {
    static ifstream fs;
    fs.open(filename);
    currentFileStream = &fs;
    
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
        if (line.find("init") == 0) { 
            processInitBlock(fs, lineNumber);
        } else if (line.find("if ") == 0) {
            processIfBlock(fs, line.substr(3), lineNumber);
        } else if (line.find("item 使用效果") == 0) {
            processItemEffectBlock(fs, line, lineNumber);
        } else {
            throw runtime_error("顶层命令必须位于init/if/item块中: " + line);
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

/*
void GameEngine::showDialog(const string& npcName, const string& dialog) {
    clear();
    printw("【%s】: %s", npcName.c_str(), dialog.c_str());
    refresh();
    getch();
}
*/

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

void GameEngine::drawUI() {
    // 绘制对话框
    if (currentDialog) {
        int startY = viewportH + 1;
        attron(A_REVERSE);
        mvprintw(startY, 0, "【%s】:", currentDialog->speaker.c_str());
        for (size_t i = 0; i < currentDialog->lines.size(); ++i) {
            mvprintw(startY + 1 + i, 0, "%s", currentDialog->lines[i].c_str());
        }
        attroff(A_REVERSE);
    }

    // 绘制物品栏
    if (gameState == GameState::INVENTORY) {
        int startX = viewportW + 2;
        attron(A_BOLD);
        mvprintw(0, startX, "=== 物品栏 ===");
        int idx = 0;
        for (const auto& item : inventory) {
            if (idx == selectedInventoryIndex) {
                attron(A_REVERSE);
                mvprintw(idx + 1, startX, "> %s", item.c_str());
                attroff(A_REVERSE);
            } else {
                mvprintw(idx + 1, startX, "  %s", item.c_str());
            }
            idx++;
        }
        attroff(A_BOLD);
    }
}

void GameEngine::draw() {
    clear();
    GameMap& curMap = maps[currentMap];
    
    // 绘制视口范围内的地图
    for (int y = 0; y < viewportH; ++y) {
        for (int x = 0; x < viewportW; ++x) {
            int mapX = viewportX + x;
            int mapY = viewportY + y;
            if (mapX >= 0 && mapX < curMap.getWidth() && 
                mapY >= 0 && mapY < curMap.getHeight()) {
                GameObject obj = curMap.getObject(mapX, mapY);
                if (obj.display != ' ') {
                    mvaddch(y, x, obj.display);
                }
            }
        }
    }

    // 绘制玩家（相对视口位置）
    int playerScreenX = playerX - viewportX;
    int playerScreenY = playerY - viewportY;
    char playerChar = '>';
    switch(playerDir) {
        case 'u': playerChar = '^'; break;
        case 'd': playerChar = 'v'; break;
        case 'l': playerChar = '<'; break;
        case 'r': playerChar = '>'; break;
    }
    if (playerScreenX >= 0 && playerScreenX < viewportW &&
        playerScreenY >= 0 && playerScreenY < viewportH) {
        mvaddch(playerScreenY, playerScreenX, playerChar);
    }

    drawUI();
    refresh();
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
    if(obj.type == "item" && obj.getProperty("可拾取", 0) == 1) {
        inventory.insert(obj.name);
        getCurrentMap().removeObject(x, y);
    }
}

void GameEngine::updateViewport() {
    viewportX = std::max(0, std::min(playerX - viewportW/2, 
        maps[currentMap].getWidth() - viewportW));
    viewportY = std::max(0, std::min(playerY - viewportH/2, 
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
    
    // 执行所有效果命令
    for (const string& effect : item.useEffects) {
        vector<string> tokens = tokenize(effect);
        runCommand(tokens);
    }
    
    // 消耗型物品使用后从背包移除
    if (item.getProperty("consumable", 0)) {
        inventory.erase(itemName);
        showDialog("系统", "已使用 " + itemName);
    }
}

void GameEngine::discardItem(const string& itemName) {
    if (inventory.erase(itemName)) {
        // 在地图上创建物品
        GameObject item = items[itemName];
        item.x = playerX;
        item.y = playerY;
        getCurrentMap().setObject(playerX, playerY, item);
        showDialog("系统", "已丢弃 " + itemName);
    }
}

std::ifstream* GameEngine::getCurrentFileStream() {
    return currentFileStream;
}

int& GameEngine::getCurrentLineNumber() {
    return currentLineNumber;
}