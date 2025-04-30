// File: src/GameEngine/Commands/ConcreteCommands/TriggerCommand.h
#pragma once
#include "CommandHandler.h"

class TriggerCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;

private:
    void handleNpcInteract(const std::vector<std::string>& args, GameEngine& engine);
};