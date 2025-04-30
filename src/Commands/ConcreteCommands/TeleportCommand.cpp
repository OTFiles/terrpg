// File: src/GameEngine/Commands/ConcreteCommands/TeleportCommand.cpp
#include "TeleportCommand.h"
#include "../GameEngine.h"
#include <stdexcept>

void TeleportCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    // 参数验证
    if (args.size() < 4) {
        throw std::runtime_error("Usage: /teleport <map> <x> <y>");
    }

    // 解析参数
    const std::string& mapName = args[1];
    auto [x, y] = CommandUtils::parseCoordinates(args, 2);

    // 地图存在性检查
    if (!engine.getMaps().count(mapName)) {
        throw std::runtime_error("地图不存在: " + mapName);
    }

    // 执行传送逻辑
    engine.setCurrentMap(mapName);
    engine.setPlayerX(x);
    engine.setPlayerY(y);

    // 显示反馈（可选，根据设计决定是否保留）
    engine.showDialog("系统", "已传送到 " + mapName + 
                      " (" + std::to_string(x) + "," + std::to_string(y) + ")");
}