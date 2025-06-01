// include/GameEngine/GameObject.h
#pragma once
#include <string>
#include <map>
#include <vector>
#include <variant> // 用于多类型属性值

/**
 * @class GameObject
 * @brief 游戏对象类，表示游戏中所有可交互的实体
 * 
 * 功能特点：
 * - 支持基础属性（位置、显示字符、名称、类型）
 * - 动态属性系统（支持多种数据类型）
 * - 对话系统（存储NPC对话内容）
 * - 使用效果系统（定义物品使用时的命令）
 */
class GameObject {
public:
    // 基本属性
    int x = 0;             ///< 对象在地图上的横坐标
    int y = 0;             ///< 对象在地图上的纵坐标
    char display = ' ';    ///< 对象在游戏地图上的显示字符
    std::string name;      ///< 对象的唯一标识名称
    std::string type;      ///< 对象类型（如"npc", "item", "wall"等）
    
    /**
     * @brief 属性值类型（支持多种数据类型）
     * 
     * 支持的数据类型包括：
     * - int: 整数值（如物品数量）
     * - float: 浮点值（如耐久度）
     * - std::string: 字符串值（如描述文本）
     * - bool: 布尔值（如状态开关）
     */
    using PropertyValue = std::variant<int, float, std::string, bool>;
    
    /**
     * @brief 动态属性存储
     * 
     * 键值对形式存储对象的附加属性：
     * - "walkable": bool - 是否可通行
     * - "count": int - 物品数量
     * - "damage": int - 武器伤害值
     * - 其他自定义属性
     */
    std::map<std::string, PropertyValue> properties;
    
    /**
     * @brief NPC对话内容
     * 
     * 键值对形式存储对话内容：
     * - "default": 默认对话
     * - "condition": 满足条件时触发的对话
     * 
     * 示例：
     * dialogues["default"] = "你好，旅行者！";
     * dialogues["have_key"] = "你找到了钥匙！";
     */
    std::map<std::string, std::string> dialogues;
    
    /**
     * @brief 物品使用效果
     * 
     * 存储物品使用时执行的游戏命令序列：
     * - 每个字符串代表一个完整的游戏命令
     * - 按顺序执行
     * 
     * 示例：
     * {"add health 10", "remove item potion"}
     */
    std::vector<std::string> useEffects;
    
    // 属性操作方法
    
    /**
     * @brief 设置属性值
     * @param key 属性名称
     * @param value 属性值（自动匹配类型）
     */
    void setProperty(const std::string& key, const PropertyValue& value);
    
    /**
     * @brief 获取属性值（模板方法）
     * @tparam T 期望的属性类型
     * @param key 属性名称
     * @param defaultValue 属性不存在时的默认值
     * @return 属性值或默认值
     * 
     * 使用示例：
     * int count = obj.getProperty<int>("count", 1);
     * bool walkable = obj.getProperty<bool>("walkable", true);
     * 
     * 注意：类型不匹配时返回默认值
     */
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
    
    /**
     * @brief 检查对象类型
     * @param typeName 类型名称
     * @return 是否匹配指定类型
     */
    bool isType(const std::string& typeName) const { return type == typeName; }
    
    /**
     * @brief 检查属性是否存在
     * @param key 属性名称
     * @return 是否存在该属性
     */
    bool hasProperty(const std::string& key) const { return properties.count(key) > 0; }
    
    // 显示相关
    
    /**
     * @brief 设置显示字符
     * @param c 代表对象的字符
     */
    void setDisplay(char c) { display = c; }
    
    /**
     * @brief 获取显示字符
     * @return 当前显示字符
     */
    char getDisplay() const { return display; }
    
    /**
     * @brief 格式化显示所有属性
     * @return 属性汇总字符串
     * 
     * 格式示例：
     * "count:5 walkable:true description:大门"
     */
    std::string getFormattedProperties() const;
};