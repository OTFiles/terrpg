#include "GameObject.h"
#include <stdexcept>

namespace terrpg::core {

GameObject::GameObject(Type type) : type_(type) {
    // 设置默认显示字符
    switch(type) {
        case Type::NPC:    displayChar_ = '@'; break;
        case Type::ITEM:   displayChar_ = '$'; break;
        case Type::TRAP:   displayChar_ = '^'; break;
        case Type::MARKER: displayChar_ = '*'; break;
        default:           displayChar_ = '?';
    }
}

void GameObject::setProperty(const String& key, int value) {
    if (key.empty()) {
        throw terrpg::common::GameException("Property key cannot be empty");
    }
    properties_[key] = value;
}

int GameObject::getProperty(const String& key, int defaultValue) const {
    auto it = properties_.find(key);
    return (it != properties_.end()) ? it->second : defaultValue;
}

bool GameObject::hasProperty(const String& key) const {
    return properties_.find(key) != properties_.end();
}

void GameObject::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

std::pair<int, int> GameObject::getPosition() const {
    return {x_, y_};
}

void GameObject::move(int dx, int dy) {
    x_ += dx;
    y_ += dy;
}

bool GameObject::checkCollision(const GameObject& other) const {
    return (x_ == other.x_) && (y_ == other.y_);
}

GameObject::Type GameObject::getType() const {
    return type_;
}

void GameObject::setDisplayChar(char c) {
    if (c < 32 || c > 126) {
        throw terrpg::common::GameException("Invalid display character");
    }
    displayChar_ = c;
}

char GameObject::getDisplayChar() const {
    return displayChar_;
}

void GameObject::triggerEvent(const String& eventName) {
    // TODO: 与ScriptSystem集成
    // 预留事件触发接口
}

} // namespace terrpg::core