// File: src/GameEngine/LoadSave.cpp
#include "GameEngine.h"
#include <fstream>
#include <regex>
#include <stdexcept>

static std::ifstream* currentFileStream = nullptr;
static int currentLineNumber = 0;

std::ifstream* GameEngine::getCurrentFileStream() { return currentFileStream; }
int& GameEngine::getCurrentLineNumber() { return currentLineNumber; }

void GameEngine::processInitBlock(std::ifstream& fs, int& lineNumber) {
    int blockDepth = 1;
    std::string line;
    while (blockDepth > 0 && getline(fs, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        if (line == "{") blockDepth++;
        else if (line == "}") blockDepth--;
        else parseLine(line);
    }
    if (blockDepth != 0) throw std::runtime_error("Unclosed init block");
}

void GameEngine::processItemEffectBlock(std::ifstream& fs, const std::string& headerLine, int& lineNumber) {
    std::vector<std::string> tokens = tokenize(headerLine);
    if(tokens.size() < 3 || tokens[2].back() != ':')
        throw std::runtime_error("Invalid item effect format: " + headerLine);
    
    std::string itemName = tokens[2].substr(0, tokens[2].size()-1);
    if(items.find(itemName) == items.end())
        throw std::runtime_error("Undefined item: " + itemName);

    int blockDepth = 1;
    std::string line;
    while (blockDepth > 0 && getline(fs, line)) {
        lineNumber++;
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if(line == "{") blockDepth++;
        else if(line == "}") blockDepth--;
        else items[itemName].useEffects.push_back(line);
    }
}

void GameEngine::loadGame(const std::string& filename) {
    static std::ifstream fs;
    fs.open(filename);
    currentFileStream = &fs;
    
    if (!fs.is_open()) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (getline(fs, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        if (line.find("init") == 0) processInitBlock(fs, lineNumber);
        else if (line.find("if ") == 0) processIfBlock(fs, line.substr(3), lineNumber);
        else if (line.find("item 使用效果") == 0) processItemEffectBlock(fs, line, lineNumber);
        else throw std::runtime_error("Top-level commands must be in init/if/item blocks: " + line);
    }

    if (!maps.count("main")) throw std::runtime_error("Missing 'main' start map");
    currentMap = "main";
}