// File: src/GameEngine/Commands/ConcreteCommands/MapCommand.cpp
#include "MapCommand.h"
#include "CommandUtils.h"
#include <sstream>

void MapCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 2) throw std::runtime_error("Invalid map command");
    
    const std::string& subcmd = args[1];
    if (subcmd == "create") {
        handleCreate(args, engine);
    } else if (subcmd == "setblock") {
        handleSetBlock(args, engine);
    } else if (subcmd == "fill") {
        handleFill(args, engine);
    }
}

void MapCommand::handleCreate(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) throw std::runtime_error("Usage: /map create <name> [width=20] [height=20]");
    
    std::string name = args[2];
    int width = 20, height = 20;
    auto params = CommandUtils::parseNamedParams(args, 3);
    
    if (params.count("width")) width = std::stoi(params["width"]);
    if (params.count("height")) height = std::stoi(params["height"]);
    
    engine.getMaps()[name] = GameMap(width, height);
    engine.getDialogSystem().showDialog({
        {"地图 " + name + " 创建成功"},
        "系统"
    });
}

void MapCommand::handleSetBlock(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 6) throw std::runtime_error("Usage: /map setblock <map> <x> <y> <type> [options...]");
    
    std::string mapName = args[2];
    auto [x, y] = CommandUtils::parseCoordinates(args, 3);
    std::string type = args[5];
    auto params = CommandUtils::parseNamedParams(args, 6);
    
    GameObject obj;
    if (type == "item") {
        if (!engine.getItems().count(params["name"])) {
            throw std::runtime_error("未定义的物品: " + params["name"]);
        }
        obj = engine.getItems()[params["name"]];
    } else {
        obj.type = type;
    }
    obj.x = x;
    obj.y = y;
    
    if (params.count("name")) obj.name = params["name"];
    
    // 设置显示字符
    if (params.count("display")) {
        obj.display = params["display"][0];
    } else {
        static std::unordered_map<std::string, char> defaults = {
            {"wall", '#'}, {"npc", '@'}, {"item", '$'},
            {"trap", '^'}, {"marker", '*'}
        };
        obj.display = defaults.count(type) ? defaults[type] : '?';
    }
    
    // 设置属性
    if (type == "wall") {
        obj.setProperty("walkable", 0);
    } else if (type == "trap") {
        int damage = params.count("damage") ? std::stoi(params["damage"]) : 10;
        obj.setProperty("damage", damage);
        obj.setProperty("walkable", 1);
    }
    
    engine.getMaps()[mapName].setObject(x, y, obj);
}

void MapCommand::handleFill(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 7) throw std::runtime_error("Usage: /map fill <map> <from x y> <to x y> <type> [options...]");
    
    std::string mapName = args[2];
    auto [x1, y1] = CommandUtils::parseCoordinates(args, 3);
    auto [x2, y2] = CommandUtils::parseCoordinates(args, 4);
    std::string type = args[5];
    auto params = CommandUtils::parseNamedParams(args, 6);
    
    GameObject obj;
    if (type == "item") {
        if (!engine.getItems().count(params["name"])) {
            throw std::runtime_error("未定义的物品: " + params["name"]);
        }
        obj = engine.getItems()[params["name"]];
    } else {
        obj.type = type;
    }
    
    if (params.count("name")) obj.name = params["name"];
    
    // 设置显示字符
    if (params.count("display")) {
        obj.display = params["display"][0];
    } else {
        static std::unordered_map<std::string, char> defaults = {
            {"wall", '#'}, {"trap", '^'}
        };
        obj.display = defaults.count(type) ? defaults[type] : '?';
    }
    
    // 设置属性
    if (type == "wall") {
        obj.setProperty("walkable", 0);
    } else if (type == "trap") {
        int damage = params.count("damage") ? std::stoi(params["damage"]) : 10;
        obj.setProperty("damage", damage);
        obj.setProperty("walkable", 1);
    }
    
    // 填充区域
    for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
        for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
            engine.getMaps()[mapName].setObject(x, y, obj);
        }
    }
    
    engine.getDialogSystem().showDialog({
        {"填充操作完成"},
        "系统"
    });
}