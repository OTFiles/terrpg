// File: src/GameEngine/Commands.cpp
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

using namespace std;

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