// include/GameEngine/InputHandler.h
#pragma once

class GameEngine;

/**
 * @class InputHandler
 * @brief 游戏输入处理器，负责根据游戏状态分发和处理玩家输入
 * 
 * 采用状态模式设计，针对不同游戏状态（探索/背包/物品操作/对话）
 * 实现差异化的输入处理逻辑。
 */
class InputHandler {
    GameEngine& engine; ///< 游戏引擎引用，用于访问游戏状态
    
public:
    /**
     * @brief 构造函数
     * @param eng 游戏引擎实例的引用
     */
    explicit InputHandler(GameEngine& eng) : engine(eng) {}
    
    /**
     * @brief 处理玩家输入
     * @param key 按下的键值（ncurses定义的KEY_*常量）
     * 
     * 根据当前游戏状态将输入分发到对应的处理方法：
     * - 探索状态 → handleExploring()
     * - 背包状态 → handleInventory()
     * - 物品操作状态 → handleItemOption()
     * - 对话状态 → handleDialog()
     */
    void processInput(int key);

private:
    // 状态特定的输入处理方法
    
    /**
     * @brief 处理探索状态输入
     * @param key 按下的键值
     * 
     * 支持操作：
     * - 方向键: 玩家移动
     * - 'g': 拾取当前物品
     * - 'u': 与NPC对话
     * - 'i': 打开背包
     * - 'q': 退出游戏
     */
    void handleExploring(int key);
    
    /**
     * @brief 处理背包状态输入
     * @param key 按下的键值
     * 
     * 支持操作：
     * - 方向键: 切换选中物品
     * - 回车: 打开物品操作菜单
     * - ESC: 返回探索状态
     */
    void handleInventory(int key);
    
    /**
     * @brief 处理物品操作状态输入
     * @param key 按下的键值
     * 
     * 支持操作：
     * - 方向键: 切换"使用/丢弃"选项
     * - 回车: 执行选中操作
     * - ESC: 返回背包状态
     */
    void handleItemOption(int key);
    
    /**
     * @brief 处理对话状态输入
     * @param key 按下的键值
     * 
     * 行为规则：
     * - 任何按键关闭对话框
     * - 返回探索状态
     */
    void handleDialog(int key);
    
    /**
     * @brief 处理移动逻辑
     * @param dx X轴移动方向（-1/0/1）
     * @param dy Y轴移动方向（-1/0/1）
     * 
     * 功能说明：
     * - 检查目标位置是否可通行
     * - 更新玩家位置
     * - 刷新游戏视口
     */
    void handleMovement(int dx, int dy);
};