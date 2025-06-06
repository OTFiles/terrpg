// File: src/GameEngine/Commands/ConcreteCommands/NpcCommand.cpp
#include "NpcCommand.h"
#include "CommandUtils.h"
#include <sstream>

void NpcCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 2) throw std::runtime_error("Usage: /npc <create|setdialogue|...>");
    
    const std::string& subcmd = args[1];
    if (subcmd == "create") handleCreate(args, engine);
    else if (subcmd == "setdialogue") handleSetDialogue(args, engine);
    else throw std::runtime_error("未知子命令: " + subcmd);
}

void NpcCommand::handleCreate(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) throw std::runtime_error("Usage: /npc create <name> [template=default]");
    
    std::string name = args[2];
    auto params = CommandUtils::parseNamedParams(args, 3);
    
    GameObject npc;
    npc.type = "npc";
    npc.name = name;
    npc.display = '@';
    
    // 应用模板（如果指定）
    if (params.count("template")) {
        const std::string& tpl = params["template"];
        if (engine.getNpcs().count(tpl)) {
            npc = engine.getNpcs().at(tpl);
            npc.name = name; // 保留指定名称
        }
    }
    
    engine.getNpcs()[name] = npc;
#ifdef DEBUG
    Log log("debug.log");
    log.debug("NPC ", name, "创建成功");
#endif
}

void NpcCommand::handleSetDialogue(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 5) throw std::runtime_error("Usage: /npc setdialogue <name> <condition> <dialogue>");
    
    std::string name = args[2];
    std::string condition = args[3];
    
    // 合并对话内容
    std::string dialogue;
    for (size_t i = 4; i < args.size(); ++i) {
        if (i > 4) dialogue += " ";
        dialogue += args[i];
    }
    
    // 检查NPC是否存在
    if (!engine.getNpcs().count(name)) {
        throw std::runtime_error("NPC不存在: " + name);
    }
    
    // 设置对话内容
    engine.getNpcs()[name].dialogues[condition] = dialogue;
#ifdef DEBUG
    Log log("debug.log");
    log.debug("已为NPC", name, "设置对话条件: ", condition);
#endif
}