// File: src/GameEngine/Commands/CommandParser.cpp
#include "CommandParser.h"
#include "ConcreteCommands/MapCommand.h"
#include "ConcreteCommands/NpcCommand.h"
#include "ConcreteCommands/ItemCommand.h"
#include "ConcreteCommands/EntityCommand.h"
#include "ConcreteCommands/TeleportCommand.h"
#include "ConcreteCommands/TriggerCommand.h"
#include "ConcreteCommands/ScoreboardCommand.h"

// 单例初始化
CommandParser& CommandParser::getInstance() {
    static CommandParser instance;
    return instance;
}

// 构造函数中的命令注册
CommandParser::CommandParser() {
    registerCommand("/map", std::make_unique<MapCommand>());
    registerCommand("/npc", std::make_unique<NpcCommand>());
    registerCommand("/item", std::make_unique<ItemCommand>());
    registerCommand("/entity", std::make_unique<>());
    registerCommand("/teleport", std::make_unique<>());
    registerCommand("/trigger", std::make_unique<>());
    registerCommand("/scoreboard", std::make_unique<>());
}

// 命令执行逻辑
void CommandParser::executeCommand(const std::string& commandLine, GameEngine& engine) {
    // 1. 解析命令为tokens（保持原逻辑）
    std::vector<std::string> tokens;
    std::stringstream ss(commandLine);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    if (tokens.empty()) return;

    // 2. 查找并执行命令处理器
    try {
        auto it = commands.find(tokens[0]);
        if (it != commands.end()) {
            it->second->handle(tokens, engine); // 多态调用
        } else {
            engine.showDialog("系统", "未知命令: " + tokens[0]);
        }
    } catch (const std::exception& e) {
        engine.showDialog("错误", std::string("命令执行失败: ") + e.what());
    }
}