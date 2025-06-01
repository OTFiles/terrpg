// include/GameEngine/InventoryManager.h
#pragma once
#include "GameObject.h"
#include <list>
#include <vector>

class GameEngine; // 前向声明

/**
 * @class InventoryManager
 * @brief 背包管理系统，负责管理玩家物品的存储、操作和使用
 * 
 * 主要功能：
 * - 物品的添加、移除和堆叠
 * - 物品的使用效果执行
 * - 物品的丢弃处理
 * - 物品的查找和选择
 */
class InventoryManager {
private:
    std::list<GameObject> items;  ///< 物品存储容器（使用list支持高效移除）
    int selectedIndex = 0;         ///< 当前选中的物品索引
    int itemInstanceCounter = 0;   ///< 物品实例ID计数器（确保每个物品有唯一标识）
    
public:
    // ================= 物品操作 =================
    
    /**
     * @brief 添加物品到背包
     * @param item 要添加的游戏对象
     * 
     * 添加规则：
     * 1. 检查物品的"stackable"属性
     * 2. 可堆叠物品：增加现有物品数量
     * 3. 不可堆叠物品：创建新物品实例
     * 4. 为每个物品生成唯一实例ID
     */
    void addItem(const GameObject& item);
    
    /**
     * @brief 移除指定物品
     * @param item 要移除的游戏对象
     * 
     * 根据实例ID精确匹配移除物品
     */
    void removeItem(const GameObject& item);
    
    /**
     * @brief 清空背包
     */
    void clear() { items.clear(); }
    
    /**
     * @brief 使用物品
     * @param item 要使用的物品
     * @param engine 游戏引擎引用
     * 
     * 使用流程：
     * 1. 执行物品的"use_effects"属性中的命令
     * 2. 消耗品减少数量（数量为0时移除）
     */
    void useItem(GameObject& item, GameEngine& engine);
    
    /**
     * @brief 丢弃物品
     * @param item 要丢弃的物品
     * @param engine 游戏引擎引用
     * 
     * 丢弃流程：
     * 1. 在玩家当前位置生成物品
     * 2. 减少物品数量（数量为0时从背包移除）
     */
    void discardItem(GameObject& item, GameEngine& engine);
    
    // ================= 物品查询 =================
    
    /**
     * @brief 获取所有物品（可修改）
     * @return 物品列表的引用
     */
    std::list<GameObject>& getItems() { return items; }
    
    /**
     * @brief 获取所有物品（只读）
     * @return 物品列表的常量引用
     */
    const std::list<GameObject>& getItems() const { return items; }
    
    /**
     * @brief 按名称检查物品是否存在
     * @param name 物品名称
     * @return 是否存在该物品
     */
    bool hasItem(const std::string& name) const;
    
    // ================= 选择操作 =================
    
    /**
     * @brief 获取当前选中物品索引
     * @return 选中索引值
     */
    int getSelectedIndex() const { return selectedIndex; }
    
    /**
     * @brief 设置当前选中物品索引
     * @param idx 新的选中索引
     */
    void setSelectedIndex(int idx) { selectedIndex = idx; }
    
private:
    // ================= 辅助方法 =================
    
    /**
     * @brief 生成唯一物品实例ID
     * @return 新的实例ID
     */
    int generateInstanceId() { return ++itemInstanceCounter; }
    
    /**
     * @brief 堆叠可堆叠物品
     * @param newItem 新添加的物品
     */
    void mergeStackable(GameObject& newItem);
};