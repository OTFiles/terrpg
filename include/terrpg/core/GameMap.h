#pragma once
#include "../../common/Types.h"
#include "../../common/Exceptions.h"
#include "GameObject.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace terrpg::core {

class GameMap {
public:
    // 地图层类型定义
    enum class Layer {
        GROUND,     // 地面层 (地形基础)
        OBJECT,     // 物体层 (可交互对象)
        CEILING     // 天花板层 (装饰性)
    };

    // 构造函数
    GameMap(int width = 20, int height = 20);
    
    // 基础属性
    int getWidth() const noexcept;
    int getHeight() const noexcept;
    
    // 对象管理
    void addObject(std::unique_ptr<GameObject> obj);
    void removeObject(uint32_t objectId);
    GameObject* getObjectAt(int x, int y, Layer layer = Layer::OBJECT) const;
    
    // 碰撞检测
    bool isWalkable(int x, int y) const;
    
    // 区域操作
    void fillArea(int x1, int y1, int x2, int y2, 
                 GameObject::Type type, 
                 const String& name = "",
                 char displayChar = ' ');
    
    // 序列化支持
    ByteStream serialize() const;
    void deserialize(const ByteStream& data);

private:
    // 分层存储结构
    struct Cell {
        std::unique_ptr<GameObject> ground;
        std::unique_ptr<GameObject> object;
        std::unique_ptr<GameObject> ceiling;
    };

    // 核心数据结构
    const int width_;
    const int height_;
    std::vector<std::vector<Cell>> grid_;
    
    // 快速查找表
    std::unordered_map<uint32_t, GameObject*> objectRegistry_;
    uint32_t nextObjectId_ = 1;

    // 内部方法
    bool isValidPosition(int x, int y) const;
    Cell& getCell(int x, int y);
    const Cell& getCell(int x, int y) const;
    GameObject* placeInLayer(Layer layer, std::unique_ptr<GameObject> obj);
};

} // namespace terrpg::core