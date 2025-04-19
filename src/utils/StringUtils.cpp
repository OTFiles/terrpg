// src/utils/StringUtils.cpp
#include "utils/StringUtils.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

using namespace terrpg;
using namespace terrpg::utils;

Vector<String> StringUtils::split(StringView str, char delimiter) {
    Vector<String> tokens;
    std::stringstream ss(str.data());
    String token;
    while (getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(trim(token));
        }
    }
    return tokens;
}

String StringUtils::trim(StringView str) {
    auto start = str.find_first_not_of(" \t");
    if (start == StringView::npos) return "";
    
    auto end = str.find_last_not_of(" \t");
    return String(str.substr(start, end - start + 1));
}

String StringUtils::toLower(StringView str) {
    String result(str);
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

String StringUtils::replaceVariables(StringView str, const Map<String, int>& variables) {
    static const std::regex varRegex(R"(\{(\w+)\})");
    String result(str);
    
    std::sregex_iterator it(result.begin(), result.end(), varRegex);
    std::sregex_iterator end;
    
    while (it != end) {
        std::smatch match = *it;
        String varName = match[1].str();
        auto varIt = variables.find(varName);
        if (varIt != variables.end()) {
            result.replace(match.position(), match.length(), std::to_string(varIt->second));
        }
        ++it;
    }
    return result;
}

bool StringUtils::startsWith(StringView str, StringView prefix) {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
}

bool StringUtils::endsWith(StringView str, StringView suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int StringUtils::toInt(StringView str, bool& success) {
    try {
        size_t pos;
        int value = std::stoi(String(str), &pos);
        success = (pos == str.size());
        return value;
    } catch (...) {
        success = false;
        return 0;
    }
}