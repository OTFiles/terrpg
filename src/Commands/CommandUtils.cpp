// src/Commands/CommandUtils.cpp
#include "CommandUtils.h"
#include <sstream>

using namespace std;

unordered_map<string, string> CommandUtils::parseNamedParams(const vector<string>& args, size_t start) {
    unordered_map<string, string> params;
    for (size_t i = start; i < args.size(); ++i) {
        size_t eqPos = args[i].find('=');
        if (eqPos != string::npos) {
            string key = args[i].substr(0, eqPos);
            string value = args[i].substr(eqPos + 1);
            params[key] = value;
        }
    }
    return params;
}

pair<int, int> CommandUtils::parseCoordinates(const vector<string>& args, size_t index) {
    try {
        // 尝试x,y格式
        if (args[index].find(',') != string::npos) {
            vector<string> parts;
            stringstream ss(args[index]);
            string part;
            while (getline(ss, part, ',')) {
                parts.push_back(part);
            }
            if (parts.size() >= 2) {
                return {stoi(parts[0]), stoi(parts[1])};
            }
        }
        // 尝试x y格式
        if (index + 1 < args.size()) {
            return {stoi(args[index]), stoi(args[index+1])};
        }
    } catch (...) {
        throw runtime_error("Invalid coordinates format");
    }
    throw runtime_error("Not enough coordinates provided");
}