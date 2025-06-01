// include/GameEngine/GameMap.h
#pragma once
#include "GameObject.h"
#include <vector>
#include <map>
#include <utility>

/**
 * @class GameMap
 * @brief 游戏地图类，负责管理游戏世界中的地图数据和对象
 * 
 * 主要功能：
 * - 存储地图尺寸和所有游戏对象
 * - 提供对象的位置查询和操作接口
 * - 实现地形可通行性检查
 * - 支持区域填充操作
 */
class GameMap {
private:
    int width;  ///< 地图宽度（单位：格子）
    int height; ///< 地图高度（单位：格子）
    
    /**
     * @brief 地图对象存储容器
     * 
     * 使用坐标对 (x,y) 作为键值存储游戏对象
     * 特点：
     * - 每个坐标位置最多一个对象
     * - 支持高效的位置查询
     * - 允许按名称查找对象
     */
    std::map<std::pair<int, int>, GameObject> objects;

public:
    /**
     * @brief 构造函数
     * @param w 地图宽度（默认20）
     * @param h 地图高度（默认20）
     */
    explicit GameMap(int w = 20, int h = 20);
    
    // 基本对象操作
    
    /**
     * @brief 在指定位置放置对象
     * @param x 横坐标
     * @param y 纵坐标
     * @param obj 要放置的游戏对象
     * 
     * 注意：会覆盖该位置原有对象
     */
    void setObject(int x, int y, const GameObject& obj);
    
    /**
     * @brief 获取指定位置的对象
     * @param x 横坐标
     * @param y 纵坐标
     * @return 该位置的游戏对象（空对象表示位置无内容）
     */
    GameObject getObject(int x, int y) const;
    
    /**
     * @brief 移除指定位置的对象
     * @param x 横坐标
     * @param y 纵坐标
     */
    void removeObject(int x, int y);
    
    // 对象查询功能
    
    /**
     * @brief 检查指定位置是否存在对象
     * @param x 横坐标
     * @param y 纵坐标
     * @return 是否存在对象
     */
    bool hasObject(int x, int y) const;
    
    /**
     * @brief 按名称检查对象是否存在
     * @param name 对象名称
     * @return 是否存在该名称的对象
     */
    bool hasObject(const std::string& name) const;
    
    /**
     * @brief 按名称获取对象
     * @param name 对象名称
     * @return 匹配名称的游戏对象（空对象表示未找到）
     * 
     * 注意：如果多个同名对象存在，返回第一个找到的对象
     */
    GameObject getObjectByName(const std::string& name) const;
    
    // 地形功能
    
    /**
     * @brief 检查位置是否可通行
     * @param x 横坐标
     * @param y 纵坐标
     * @return 是否可通行
     * 
     * 判断规则：
     * 1. 坐标超出地图边界 → 不可通行
     * 2. 存在"wall"类型对象 → 不可通行
     * 3. 对象walkable属性为false → 不可通行
     * 4. 其他情况 → 可通行
     */
    bool isWalkable(int x, int y) const;
    
    // 地图信息获取
    
    /**
     * @brief 获取地图宽度
     * @return 宽度值
     */
    int getWidth() const { return width; }
    
    /**
     * @brief 获取地图高度
     * @return 高度值
     */
    int getHeight() const { return height; }
    
    /**
     * @brief 获取所有对象
     * @return 对象集合的常量引用
     */
    const std::map<std::pair<int, int>, GameObject>& getAllObjects() const { return objects; }
    
    // 批量操作
    
    /**
     * @brief 填充矩形区域
     * @param x1,y1 区域左上角坐标
     * @param x2,y2 区域右下角坐标
     * @param templateObj 用作模板的游戏对象
     * 
     * 功能说明：
     * - 自动处理坐标顺序（无需左上/右下顺序）
     * - 为区域内每个格子创建模板对象的副本
     * - 设置对象的正确坐标位置
     */
    void fillArea(int x1, int y1, int x2, int y2, const GameObject& templateObj);
};