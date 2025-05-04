// File: src/GameEngine/Commands/ConcreteCommands/TriggerCommand.cpp
#include "TriggerCommand.h"
#include "../CommandUtils.h"
#include "../../GameEngine.h"

void TriggerCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) {
        throw std::runtime_error("Usage: /trigger <event> [args...]");
    }

    const std::string& event = args[1];
    
    if (event == "npc.interact") {
        handleNpcInteract(args, engine);
    } else {
        throw std::runtime_error("未知事件: " + event);
    }
}

void TriggerCommand::handleNpcInteract(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) {
        throw std::runtime_error("Usage: /trigger npc.interact <name> [condition]");
    }

    std::string name = args[2];
    std::string condition = args.size() > 3 ? args[3] : "always";

    auto& npcs = engine.getNpcs();
    if (!npcs.count(name)) {
        throw std::runtime_error("NPC不存在: " + name);
    }

    auto& npc = npcs[name];
    if (npc.dialogues.count(condition)) {
        engine.showDialog(name, npc.dialogues.at(condition));
    } else {
        engine.showDialog(name, "...");
    }
}