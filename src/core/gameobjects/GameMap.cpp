#include "GameMap.h"
#include <algorithm>
#include <stdexcept>

namespace terrpg::core {

GameMap::GameMap(int width, int height) 
    : width_(std::max(1, width)), 
      height_(std::max(1, height)),
      grid_(height_, std::vector<Cell>(width_)) {}

int GameMap::getWidth() const noexcept {
    return width_;
}

int GameMap::getHeight() const noexcept {
    return height_;
}

bool GameMap::isValidPosition(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

GameMap::Cell& GameMap::getCell(int x, int y) {
    if(!isValidPosition(x, y)) {
        throw terrpg::common::GameException("Invalid map position: (" + 
                                          std::to_string(x) + "," + 
                                          std::to_string(y) + ")");
    }
    return grid_[y][x];
}

const GameMap::Cell& GameMap::getCell(int x, int y) const {
    return const_cast<GameMap*>(this)->getCell(x, y);
}

void GameMap::addObject(std::unique_ptr<GameObject> obj) {
    auto [x, y] = obj->getPosition();
    Cell& cell = getCell(x, y);
    
    GameObject* target = nullptr;
    switch(obj->getType()) {
        case GameObject::Type::TRAP:
        case GameObject::Type::ITEM:
            target = cell.object.get();
            break;
        case GameObject::Type::MARKER:
            target = cell.ground.get();
            break;
        default:
            target = cell.object.get();
    }

    if(target) {
        throw terrpg::common::GameException("Position occupied");
    }

    const uint32_t objectId = nextObjectId_++;
    obj->setProperty("objectId", objectId);
    GameObject* rawPtr = obj.get();
    
    switch(obj->getType()) {
        case GameObject::Type::MARKER:
            cell.ground = std::move(obj);
            break;
        case GameObject::Type::TRAP:
        case GameObject::Type::ITEM:
            cell.object = std::move(obj);
            break;
        default:
            cell.object = std::move(obj);
    }

    objectRegistry_[objectId] = rawPtr;
}

void GameMap::removeObject(uint32_t objectId) {
    auto it = objectRegistry_.find(objectId);
    if(it == objectRegistry_.end()) {
        throw terrpg::common::GameException("Object not found: " + 
                                          std::to_string(objectId));
    }

    GameObject* obj = it->second;
    auto [x, y] = obj->getPosition();
    Cell& cell = getCell(x, y);

    if(cell.ground.get() == obj) {
        cell.ground.reset();
    } else if(cell.object.get() == obj) {
        cell.object.reset();
    } else if(cell.ceiling.get() == obj) {
        cell.ceiling.reset();
    }

    objectRegistry_.erase(it);
}

GameObject* GameMap::getObjectAt(int x, int y, Layer layer) const {
    const Cell& cell = getCell(x, y);
    switch(layer) {
        case Layer::GROUND:  return cell.ground.get();
        case Layer::OBJECT:  return cell.object.get();
        case Layer::CEILING: return cell.ceiling.get();
        default: return nullptr;
    }
}

bool GameMap::isWalkable(int x, int y) const {
    if(!isValidPosition(x, y)) return false;
    
    const Cell& cell = getCell(x, y);
    if(cell.object && cell.object->getProperty("walkable", 1) == 0) {
        return false;
    }
    return true;
}

void GameMap::fillArea(int x1, int y1, int x2, int y2,
                      GameObject::Type type,
                      const String& name,
                      char displayChar) 
{
    const int minX = std::clamp(std::min(x1, x2), 0, width_-1);
    const int maxX = std::clamp(std::max(x1, x2), 0, width_-1);
    const int minY = std::clamp(std::min(y1, y2), 0, height_-1);
    const int maxY = std::clamp(std::max(y1, y2), 0, height_-1);

    for(int y = minY; y <= maxY; ++y) {
        for(int x = minX; x <= maxX; ++x) {
            auto obj = std::make_unique<GameObject>(type);
            obj->setPosition(x, y);
            obj->setDisplayChar(displayChar);
            
            if(!name.empty()) {
                obj->setProperty("name", 0); // 实际应为字符串属性，需扩展属性系统
            }

            // 设置类型特定属性
            switch(type) {
                case GameObject::Type::TRAP:
                    obj->setProperty("damage", 10);
                    obj->setProperty("walkable", 1);
                    break;
                case GameObject::Type::WALL:
                    obj->setProperty("walkable", 0);
                    break;
                default: break;
            }

            try {
                addObject(std::move(obj));
            } catch(const terrpg::common::GameException&) {
                // 忽略已占位的情况
            }
        }
    }
}

ByteStream GameMap::serialize() const {
    // TODO: 实现序列化
    return {};
}

void GameMap::deserialize(const ByteStream& data) {
    // TODO: 实现反序列化
}

} // namespace terrpg::core