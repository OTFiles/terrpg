// src/commands/MapCommands.cpp
#include "commands/MapCommands.h"
#include "core/GameEngine.h"
#include "common/Exceptions.h"
#include <stdexcept>
#include <map>

using namespace terrpg;
using namespace terrpg::commands;
using namespace terrpg::core;

void AddMapCommand::execute(GameEngine& engine, const std::vector<std::string>& tokens) {
    validateTokens(tokens, 3, "add map <地图名称>");
    const std::string& mapName = tokens[2];
    engine.maps[mapName] = GameMap(20, 20);
}

void SetMapCommand::execute(GameEngine& engine, const std::vector<std::string>& tokens) {
    validateTokens(tokens, 7, "set map <地图名> <x> <y> <类型> [名称] [显示字符]");
    
    const std::string& mapName = tokens[2];
    int x = std::stoi(tokens[3]);
    int y = std::stoi(tokens[4]);
    const std::string& type = tokens[5];
    
    GameObject obj;
    obj.x = x;
    obj.y = y;
    obj.type = type;

    // 名称处理
    static const std::set<std::string> namedTypes = {"npc", "item", "陷阱", "标记点"};
    if (namedTypes.count(type)) {
        if (tokens.size() < 7) throw common::CommandException("需要名称参数");
        obj.name = tokens[6];
    }

    // 显示字符处理
    size_t displayIndex = namedTypes.count(type) ? 7 : 6;
    if (tokens.size() > displayIndex) {
        obj.display = !tokens[displayIndex].empty() ? tokens[displayIndex][0] : ' ';
    }

    // 设置默认显示字符
    static const std::map<std::string, char> defaults = {
        {"wall", '#'}, {"npc", '@'}, {"item", '$'},
        {"陷阱", '^'}, {"标记点", '*'}
    };
    if (obj.display == ' ' && defaults.count(type)) {
        obj.display = defaults.at(type);
    }

    // 设置默认属性
    if (type == "wall") {
        obj.setProperty("walkable", 0);
    } else if (type == "陷阱") {
        obj.setProperty("damage", 10);
        obj.setProperty("walkable", 1);
    }

    // 处理物品模板
    if (type == "item" && engine.items.count(obj.name)) {
        const auto& tpl = engine.items[obj.name];
        obj.properties = tpl.properties;
        obj.useEffects = tpl.useEffects;
        if (tpl.display != ' ') obj.display = tpl.display;
    }

    engine.maps[mapName].setObject(x, y, obj);
}

void FillCommand::execute(GameEngine& engine, const std::vector<std::string>& tokens) {
    validateTokens(tokens, 8, "fill <地图名> <x1> <y1> <x2> <y2> <类型> [陷阱名称]");
    
    const std::string& mapName = tokens[1];
    auto& targetMap = engine.maps[mapName];
    
    // 解析坐标
    int coords[4];
    for (int i = 0; i < 4; ++i) {
        coords[i] = std::stoi(tokens[2 + i]);
    }
    
    // 创建填充对象
    GameObject obj;
    const std::string& type = tokens[6];
    if (type == "wall") {
        obj = createWall();
    } else if (type == "陷阱") {
        validateTokens(tokens, 8, "fill ... 陷阱 <名称>");
        obj = createTrap(tokens[7]);
    } else {
        throw common::CommandException("不支持的填充类型: " + type);
    }

    // 执行填充
    fillArea(targetMap, coords[0], coords[1], coords[2], coords[3], obj);
}

GameObject FillCommand::createWall() {
    GameObject obj;
    obj.type = "wall";
    obj.display = '#';
    obj.setProperty("walkable", 0);
    return obj;
}

GameObject FillCommand::createTrap(const std::string& name) {
    GameObject obj;
    obj.type = "trap";
    obj.display = '^';
    obj.name = name;
    obj.setProperty("damage", 10);
    obj.setProperty("walkable", 1);
    return obj;
}

void FillCommand::fillArea(GameMap& map, int x1, int y1, int x2, int y2, const GameObject& obj) {
    int minX = std::min(x1, x2);
    int maxX = std::max(x1, x2);
    int minY = std::min(y1, y2);
    int maxY = std::max(y1, y2);
    
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            if (x >= 0 && x < map.getWidth() && y >= 0 && y < map.getHeight()) {
                map.setObject(x, y, obj);
            }
        }
    }
}