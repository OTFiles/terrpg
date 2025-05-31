// include/GameObject.h
#pragma once
#include <string>
#include <map>
#include <vector>
#include <variant> // 用于多类型属性值

class GameObject {
public:
    // 基本属性
    int x = 0;
    int y = 0;
    char display = ' ';
    std::string name;
    std::string type;
    
    // 使用variant支持多种属性类型
    using PropertyValue = std::variant<int, float, std::string, bool>;
    std::map<std::string, PropertyValue> properties;
    
    // 对话系统
    std::map<std::string, std::string> dialogues;
    
    // 使用效果
    std::vector<std::string> useEffects;
    
    // 属性操作方法
    void setProperty(const std::string& key, const PropertyValue& value);
    
    template<typename T>
    T getProperty(const std::string& key, T defaultValue = T()) const {
        auto it = properties.find(key);
        if (it != properties.end()) {
            try {
                return std::get<T>(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    // 类型检查
    bool isType(const std::string& typeName) const { return type == typeName; }
    bool hasProperty(const std::string& key) const { return properties.count(key) > 0; }
    
    // 显示相关
    void setDisplay(char c) { display = c; }
    char getDisplay() const { return display; }
    
    std::string getFormattedProperties() const;
};