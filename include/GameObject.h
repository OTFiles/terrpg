// File: GameObject.h
#pragma once
#include <string>
#include <map>
#include <vector>

struct GameObject {
    int x = 0;
    int y = 0;
    char display = ' ';
    std::string name;
    std::string type;
    std::map<std::string, int> properties;
    std::map<std::string, std::string> dialogues;
    std::vector<std::string> useEffects;

    void setProperty(const std::string& key, int value);
    int getProperty(const std::string& key, int def = 0) const;
};