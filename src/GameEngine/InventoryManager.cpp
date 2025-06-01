// src/InventoryManager.cpp
#include "InventoryManager.h"
#include "GameEngine.h"
#include "Log.h"

void InventoryManager::addItem(const GameObject& item) {
    GameObject newItem = item;
    const bool stackable = newItem.getProperty("stackable", 0);
    
    // 生成唯一实例ID
    newItem.setProperty("instance_id", generateInstanceId());
    
    // 堆叠逻辑
    if(stackable) {
        for(auto& existing : items) {
            if(existing.name == newItem.name) {
                int curCount = existing.getProperty("count", 1);
                existing.setProperty("count", curCount + 1);
                return;
            }
        }
    }
    
    // 新物品
    newItem.setProperty("count", 1);
    items.push_back(newItem);
    
#ifdef DEBUG
    Log debug("debug.log");
    debug.debug("Added item:", newItem.name, 
               "Instance ID:", newItem.getProperty("instance_id", 0));
#endif
}

void InventoryManager::removeItem(const GameObject& item) {
    items.remove_if([&](const GameObject& i) {
        return i.getProperty("instance_id", 0) == item.getProperty("instance_id", 0);
    });
}

void InventoryManager::useItem(GameObject& item, GameEngine& engine) {
    // 执行使用效果
    std::string effectsStr = item.getProperty<std::string>("use_effects", "");
    std::vector<std::string> useEffects = engine.tokenize(effectsStr);
    for(const auto& effect : useEffects) {
        engine.runCommand(engine.tokenize(effect));
    }
    
    // 消耗品处理
    if(item.getProperty("consumable", 0)) {
        int count = item.getProperty("count", 1);
        if(count > 1) {
            item.setProperty("count", count - 1);
        } else {
            removeItem(item);
        }
    }
}

void InventoryManager::discardItem(GameObject& item, GameEngine& engine) {
    // 生成掉落物
    GameObject dropItem = item;
    dropItem.x = engine.getPlayerX();
    dropItem.y = engine.getPlayerY();
    engine.getCurrentMap().setObject(dropItem.x, dropItem.y, dropItem);
    
    // 从库存移除
    int count = item.getProperty("count", 1);
    if(count > 1) {
        item.setProperty("count", count - 1);
    } else {
        removeItem(item);
    }
}

bool InventoryManager::hasItem(const std::string& name) const {
    return std::any_of(items.begin(), items.end(), 
        [&](const GameObject& i){ return i.name == name; });
}