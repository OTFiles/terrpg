#pragma once
#include "CommandHandler.h"
#include <unordered_map>
#include <memory>

class CommandParser {
public:
    static CommandParser& getInstance() {
        static CommandParser instance;
        return instance;
    }

    void registerCommand(const std::string& cmd, std::unique_ptr<CommandHandler> handler) {
        commands[cmd] = std::move(handler);
    }

    void executeCommand(const std::string& commandLine, GameEngine& engine);

private:
    CommandParser();
    std::unordered_map<std::string, std::unique_ptr<CommandHandler>> commands;
};