// File: src/GameObject.cpp
#include "GameObject.h"

void GameObject::setProperty(const std::string& key, const PropertyValue& value) {
    properties[key] = value;
}

std::string GameObject::getFormattedProperties() const {
    std::string result;
    for (const auto& [key, value] : properties) {
        result += key + ":" + std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return arg;
            } else if constexpr (std::is_same_v<T, bool>) {
                return arg ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(arg);
            } else {
                return "[unprintable]";
            }
        }, value) + " ";
    }
    return !result.empty() ? result.substr(0, result.size()-1) : "无";
}