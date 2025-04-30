#pragma once
#include <vector>
#include <string>
#include <unordered_map>

class CommandUtils {
public:
    static std::unordered_map<std::string, std::string> parseNamedParams(const std::vector<std::string>& args, size_t start = 0);
    static std::pair<int, int> parseCoordinates(const std::vector<std::string>& args, size_t index);
};