// File: src/GameEngine/Commands/ConcreteCommands/TeleportCommand.h
#pragma once
#include "CommandHandler.h"
#include "CommandUtils.h"

class TeleportCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;
};