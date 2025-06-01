// File: src/GameEngine/Commands/ConcreteCommands/ScoreboardCommand.h
#pragma once
#include "CommandHandler.h"
#include "CommandUtils.h"

class ScoreboardCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;

private:
    void handleAdd(const std::vector<std::string>& args, GameEngine& engine);
    void handleSet(const std::vector<std::string>& args, GameEngine& engine);
    void handleOperation(const std::vector<std::string>& args, GameEngine& engine);
};