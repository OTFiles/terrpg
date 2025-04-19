#pragma once
#include "../../common/Types.h"
#include "../../common/Exceptions.h"
#include <map>
#include <memory>

namespace terrpg::core {

class GameObject {
public:
    // 对象类型枚举
    enum class Type {
        ENTITY,     // 基础实体
        NPC,        // 非玩家角色
        ITEM,       // 可交互物品
        TRAP,       // 陷阱
        MARKER      // 地图标记
    };

    // 构造函数
    explicit GameObject(Type type = Type::ENTITY);
    
    // 属性管理
    void setProperty(const String& key, int value);
    int getProperty(const String& key, int defaultValue = 0) const;
    bool hasProperty(const String& key) const;

    // 位置管理
    void setPosition(int x, int y);
    std::pair<int, int> getPosition() const;
    void move(int dx, int dy);

    // 碰撞检测
    bool checkCollision(const GameObject& other) const;

    // 类型系统
    Type getType() const;
    void setDisplayChar(char c);
    char getDisplayChar() const;

    // 脚本系统接口
    void triggerEvent(const String& eventName);

    // 序列化标记
    static const uint32_t SERIALIZATION_VERSION = 0x00010000; // v1.0.0.0

private:
    Type type_;
    char displayChar_ = ' ';
    int x_ = 0;
    int y_ = 0;
    String name_;
    std::map<String, int> properties_;
};

using GameObjectPtr = std::unique_ptr<GameObject>;

} // namespace terrpg::core