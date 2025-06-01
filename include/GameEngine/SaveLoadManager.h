// include/GameEngine/SaveLoadManager.h
#pragma once
#include "GameObject.h"
#include <fstream>
#include <sstream>
#include <unordered_set>

class GameEngine;

/**
 * @class SaveLoadManager
 * @brief 游戏状态存档/读档管理器，负责游戏状态的持久化存储和恢复
 * 
 * 功能特性：
 * - 支持完整游戏状态序列化/反序列化
 * - 自定义文本格式存档（人类可读）
 * - 自动处理特殊字符转义
 * - 错误处理和日志记录
 * 
 * 存档格式规范：
 * 1. 版本标识头
 * 2. 玩家位置和方向
 * 3. 游戏变量集合
 * 4. 地点访问标记
 * 5. 玩家物品栏
 * 6. 各地图对象状态
 */
class SaveLoadManager {
public:
    /**
     * @brief 保存游戏状态到文件
     * @param engine 游戏引擎引用（只读）
     * @param filename 存档文件路径
     * 
     * 保存内容包括：
     * - 玩家位置和朝向
     * - 所有游戏变量
     * - 已访问地点标记
     * - 物品栏所有物品及其属性
     * - 所有地图的对象布局
     * 
     * 错误处理：
     * - 文件创建失败显示系统提示
     * - 序列化错误记录到日志
     */
    void saveState(const GameEngine& engine, const std::string& filename);
    
    /**
     * @brief 从文件加载游戏状态
     * @param engine 游戏引擎引用（可修改）
     * @param filename 存档文件路径
     * 
     * 加载流程：
     * 1. 清空当前游戏状态
     * 2. 按顺序重建各系统状态
     * 3. 恢复玩家位置和地图对象
     * 
     * 安全机制：
     * - 文件不存在时显示错误
     * - 格式错误时回滚并提示
     * - 详细错误日志记录
     */
    void loadState(GameEngine& engine, const std::string& filename);

private:
    /**
     * @brief 序列化游戏对象
     * @param os 输出流
     * @param obj 要序列化的游戏对象
     * 
     * 序列化格式：
     * name type display x y {prop1:value1;prop2:value2}
     * 特殊字符自动转义处理
     */
    void serializeGameObject(std::ostream& os, const GameObject& obj);
    
    /**
     * @brief 反序列化游戏对象
     * @param is 输入流
     * @return 重建的游戏对象
     * 
     * 智能属性类型推断：
     * - 包含小数点作为float处理
     * - 纯数字作为int处理
     * - 其他情况作为string处理
     */
    GameObject deserializeGameObject(std::istream& is);
    
    /**
     * @brief 字符串转义处理
     * @param str 原始字符串
     * @return 转义后的字符串
     * 
     * 转义规则：
     * 空格 → \s
     * 换行 → \n
     * 反斜杠 → \\
     */
    std::string escapeString(const std::string& str);
    
    /**
     * @brief 字符串反转义处理
     * @param str 转义后的字符串
     * @return 原始字符串
     * 
     * 注意：连续反斜杠的正确解析
     */
    std::string unescapeString(const std::string& str);
};