// include/InventoryManager.h
#pragma once
#include "GameObject.h"
#include <list>
#include <vector>

class GameEngine; // 前向声明

class InventoryManager {
private:
    std::list<GameObject> items;
    int selectedIndex = 0;
    int itemInstanceCounter = 0;
    
public:
    // 物品操作
    void addItem(const GameObject& item);
    void removeItem(const GameObject& item);
    void clear() { items.clear(); }
    void useItem(GameObject& item, GameEngine& engine);
    void discardItem(GameObject& item, GameEngine& engine);
    
    // 物品查询
    std::list<GameObject>& getItems() { return items; }
    const std::list<GameObject>& getItems() const { return items; } //我也不知道为什么需要
    bool hasItem(const std::string& name) const;
    
    // 选择操作
    int getSelectedIndex() const { return selectedIndex; }
    void setSelectedIndex(int idx) { selectedIndex = idx; }
    
private:
    // 辅助方法
    int generateInstanceId() { return ++itemInstanceCounter; }
    void mergeStackable(GameObject& newItem);
};