// File: src/GameEngine/Commands/ConcreteCommands/EntityCommand.h
#pragma once
#include "CommandHandler.h"
#include <vector>
#include <string>

class EntityCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;

private:
    void handleSet(const std::vector<std::string>& args, GameEngine& engine);
};