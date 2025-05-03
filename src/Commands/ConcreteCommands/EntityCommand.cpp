// File: src/GameEngine/Commands/ConcreteCommands/EntityCommand.cpp
#include "EntityCommand.h"
#include "CommandUtils.h"
#include "../GameEngine.h"
#include "../GameMap.h"

void EntityCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) {
        throw std::runtime_error("Usage: /entity <set|get> <name> <property> [value]");
    }

    const std::string& subcmd = args[1];
    if (subcmd == "set") {
        handleSet(args, engine);
    } else {
        throw std::runtime_error("未知子命令: " + subcmd);
    }
}

void EntityCommand::handleSet(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 5) {
        throw std::runtime_error("Usage: /entity set <name> <property> <value>");
    }

    const std::string& name = args[2];
    const std::string& property = args[3];
    
    // 合并剩余参数作为值
    std::string value;
    for (size_t i = 4; i < args.size(); ++i) {
        if (i > 4) value += " ";
        value += args[i];
    }

    // 在NPC中查找
    if (engine.getNpcs().count(name)) {
        engine.getNpcs()[name].setProperty(property, value);
    }
    // 在物品中查找
    else if (engine.getItems().count(name)) {
        engine.getItems()[name].setProperty(property, value);
    }
    // 在地图对象中查找
    else {
        bool found = false;
        for (auto& [mapName, gameMap] : engine.getMaps()) {
            if (gameMap.hasObject(name)) {
                gameMap.getObjectByName(name).setProperty(property, value);
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("未找到实体: " + name);
        }
    }

    engine.showDialog("系统", "已更新实体 " + name + " 的属性 " + property);
}