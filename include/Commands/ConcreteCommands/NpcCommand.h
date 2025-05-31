// File: src/GameEngine/Commands/ConcreteCommands/NpcCommand.h
#pragma once
#include "CommandHandler.h"
#include "GameObject.h"

class NpcCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;

private:
    void handleCreate(const std::vector<std::string>& args, GameEngine& engine);
    void handleSetDialogue(const std::vector<std::string>& args, GameEngine& engine);
    
    // NPC默认属性
    inline static const std::unordered_map<std::string, char> DEFAULT_DISPLAYS = {
        {"villager", '@'}, {"guard", 'G'}, {"merchant", 'M'}
    };
};