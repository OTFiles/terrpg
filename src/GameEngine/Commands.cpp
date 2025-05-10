// File: src/GameEngine/Commands.cpp
#include "GameEngine.h"
#include "Commands/CommandParser.h"
#include <unordered_map>
#include <functional>
#include <sstream>
#include <algorithm>

void GameEngine::runCommand(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return;
    
    // 直接使用新命令系统
    std::string commandLine;
    for (const auto& token : tokens) {
        if (!commandLine.empty()) commandLine += " ";
        commandLine += token;
    }
    
    // 使用新的命令解析器
    CommandParser::parseAndExecute(commandLine, *this);
}