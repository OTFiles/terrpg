// File: src/GameEngine/Commands/ConcreteCommands/MapCommand.h
#pragma once
#include "CommandHandler.h"
#include "GameMap.h"
#include "GameObject.h"

class MapCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;
    virtual ~MapCommand() = default;

private:
    void handleCreate(const std::vector<std::string>& args, GameEngine& engine);
    void handleSetBlock(const std::vector<std::string>& args, GameEngine& engine);
    void handleFill(const std::vector<std::string>& args, GameEngine& engine);
    
    // 默认显示字符映射
    inline static const std::unordered_map<std::string, char> DEFAULT_DISPLAYS = {
        {"wall", '#'}, {"npc", '@'}, {"item", '$'},
        {"trap", '^'}, {"marker", '*'}
    };
};