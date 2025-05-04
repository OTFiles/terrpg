#pragma once
#include <vector>
#include <string>

class LegacyCommandTranslator {
public:
    static std::string translate(const std::vector<std::string>& tokens);
};