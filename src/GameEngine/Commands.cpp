// File: src/GameEngine/Commands.cpp
#include "GameEngine.h"
#include "Commands/CommandParser.h"
#include "Commands/LegacyCommandTranslator.h"
#include <unordered_map>
#include <functional>
#include <sstream>
#include <algorithm>

void GameEngine::runCommand(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return;
    
    // 将旧命令转换为新命令格式
    std::string commandLine;
    for (const auto& token : tokens) {
        if (!commandLine.empty()) commandLine += " ";
        commandLine += token;
    }
    
    // 使用新的命令解析器
    std::string translated = LegacyCommandTranslator::translate(tokens);
    if (!translated.empty()) {
        CommandParser::parseAndExecute(translated, *this);
    } else {
        CommandParser::parseAndExecute(commandLine, *this);
    }
}