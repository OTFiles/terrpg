// File: GameMap.h
#pragma once
#include "GameObject.h"
#include <vector>
#include <map>
#include <utility>

class GameMap {
private:
    int width;
    int height;
    std::map<std::pair<int, int>, GameObject> objects; // 改为map存储以便按名称查找

public:
    explicit GameMap(int w = 20, int h = 20);
    
    // 基本对象操作
    void setObject(int x, int y, const GameObject& obj);
    GameObject getObject(int x, int y) const;
    void removeObject(int x, int y);
    
    // 新增功能方法
    bool hasObject(int x, int y) const;
    bool hasObject(const std::string& name) const;
    GameObject getObjectByName(const std::string& name) const;
    
    // 地形检查
    bool isWalkable(int x, int y) const;
    
    // 获取地图信息
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    const std::map<std::pair<int, int>, GameObject>& getAllObjects() const { return objects; }
    
    // 批量操作
    void fillArea(int x1, int y1, int x2, int y2, const GameObject& templateObj);
};