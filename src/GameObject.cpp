// File: src/GameObject.cpp
#include "GameObject.h"

void GameObject::setProperty(const std::string& key, int value) {
    properties[key] = value;
}

int GameObject::getProperty(const std::string& key, int def) const {
    auto it = properties.find(key);
    return it != properties.end() ? it->second : def;
}