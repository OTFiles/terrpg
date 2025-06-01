// File: include/GameEngine/GameEngine.h
#pragma once
#include "GameState.h"
#include "GameMap.h"
#include "GameObject.h"
#include "ConditionEvaluator.h"
#include "DialogSystem.h"
#include "InventoryManager.h"
#include "SaveLoadManager.h"
#include "Renderer.h"
#include "InputHandler.h"
#include <map>
#include <set>
#include <vector>
#include <memory>

/**
 * @class GameEngine
 * @brief 游戏引擎核心类，负责协调所有子系统和游戏逻辑
 *
 * 主要职责：
 * - 管理游戏状态和全局数据
 * - 处理命令执行和脚本解析
 * - 协调各子系统交互
 * - 提供游戏对象操作接口
 *
 * 架构说明：
 * 1. 采用ECS混合架构管理游戏对象
 * 2. 基于状态模式处理不同游戏阶段
 * 3. 事件驱动与轮询混合的更新机制
 */
class GameEngine {
private:
    // 游戏核心数据
    std::map<std::string, GameMap> maps;          ///< 所有游戏地图(名称->实例)
    std::map<std::string, GameObject> npcTemplates; ///< NPC模板库
    std::map<std::string, GameObject> items;      ///< 物品定义库
    std::map<std::string, int> variables;         ///< 游戏变量存储
    std::set<std::string> visitedMarkers;         ///< 已访问地点标记

    // 子系统
    InventoryManager inventoryManager;            ///< 物品栏管理系统
    DialogSystem dialogSystem;                    ///< 对话系统
    SaveLoadManager saveLoadManager;              ///< 存档管理系统
    std::unique_ptr<Renderer> renderer;          ///< 渲染系统(拥有所有权)

    // 运行时状态
    std::string currentMap = "start";             ///< 当前地图标识
    int playerX = 5;                              ///< 玩家X坐标(地图格子单位)
    int playerY = 5;                              ///< 玩家Y坐标(地图格子单位)
    char playerDir = 'd';                         ///< 玩家朝向(u/d/l/r)
    GameState gameState = GameState::EXPLORING;   ///< 当前游戏状态
    int viewportX = 0;                            ///< 视口左上角X坐标
    int viewportY = 0;                            ///< 视口左上角Y坐标
    int viewportW = 20;                           ///< 视口宽度(格子数)
    int viewportH = 10;                           ///< 视口高度(格子数)

public:
    /**
     * @brief 构造函数
     * 初始化各子系统并设置默认状态
     */
    GameEngine();
    
    // 核心接口
    /**
     * @brief 加载游戏数据文件
     * @param filename 游戏定义文件路径
     * @throws runtime_error 文件格式错误时抛出异常
     *
     * 文件格式要求：
     * - 必须包含init块
     * - 必须定义'main'地图
     * - 支持嵌套条件块
     */
    void loadGame(const std::string& filename);
    
    /**
     * @brief 保存当前游戏状态
     * @param filename 存档文件路径
     *
     * 保存内容包括：
     * - 玩家位置和地图状态
     * - 所有游戏变量
     * - 物品栏内容
     */
    void saveGame(const std::string& filename);
    
    /**
     * @brief 启动主游戏循环
     *
     * 循环流程：
     * 1. 渲染当前帧
     * 2. 处理玩家输入
     * 3. 更新游戏状态
     */
    void startGameLoop();
    
    /**
     * @brief 执行游戏命令
     * @param tokens 已分词的命令参数
     *
     * 注意：会触发命令解析器的完整处理流程
     */
    void runCommand(const std::vector<std::string>& tokens);
    
    /**
     * @brief 显示对话内容
     * @param speaker 说话者名称
     * @param content 对话文本(可包含换行)
     */
    void showDialog(const std::string& speaker, const std::string& content);
    
    // 访问方法
    /**
     * @brief 获取当前地图(可修改)
     * @return 当前地图引用
     * @throws out_of_range 当前地图不存在时抛出异常
     */
    GameMap& getCurrentMap();
    
    /**
     * @brief 获取当前地图(只读)
     * @return 当前地图常量引用
     */
    const GameMap& getCurrentMap() const { return maps.at(currentMap); }
    
    // 子系统访问器
    InventoryManager& getInventoryManager() { return inventoryManager; }
    const InventoryManager& getInventoryManager() const { return inventoryManager; }
    DialogSystem& getDialogSystem() { return dialogSystem; }
    const DialogSystem& getDialogSystem() const { return dialogSystem; }
    std::set<std::string>& getVisitedMarkers() { return visitedMarkers; }
    const std::set<std::string>& getVisitedMarkers() const { return visitedMarkers; }
    
    // 游戏状态操作
    /**
     * @brief 更新视口位置
     * 根据玩家位置自动计算可见区域
     */
    void updateViewport();
    
    /**
     * @brief 解析并执行单行脚本
     * @param line 脚本内容(自动去除首尾空白)
     */
    void parseLine(const std::string& line);
    
    /**
     * @brief 生成物品唯一ID
     * @return 自增的物品实例ID
     */
    int generateItemInstanceId();
    
    /**
     * @brief 分词工具
     * @param line 输入字符串
     * @return 分词结果向量
     */
    std::vector<std::string> tokenize(const std::string& line);
    
    /**
     * @brief 拾取指定位置的物品
     * @param x 地图X坐标
     * @param y 地图Y坐标
     *
     * 处理逻辑：
     * - 检查物品pickupable属性
     * - 处理可堆叠物品合并
     * - 生成新物品实例ID
     */
    void pickupItem(int x, int y);
    
    /**
     * @brief 尝试与相邻NPC对话
     * 检测玩家四方向相邻位置的NPC并触发对话
     */
    void tryTalkToNPC();

    // 原始属性访问
    int& getPlayerX() { return playerX; }
    int& getPlayerY() { return playerY; }
    int getPlayerX() const { return playerX; }
    int getPlayerY() const { return playerY; }
    void setPlayerX(int x) { playerX = x; }
    void setPlayerY(int y) { playerY = y; }
    char& getPlayerDir() { return playerDir; }
    char getPlayerDir() const { return playerDir; }
    GameState& getGameState() { return gameState; }
    GameState getGameState() const { return gameState; }
    void setGameState(const GameState& state) { gameState = state; }
    int getViewportW() const { return viewportW; }
    int getViewportH() const { return viewportH; }
    
    /**
     * @brief 评估游戏条件
     * @param condition 条件表达式
     * @return 条件是否满足
     *
     * 支持的条件类型：
     * - 变量比较
     * - 物品持有检查
     * - 地点访问标记
     */
    bool evalCondition(const std::string& condition);
    
    // 数据容器访问
    std::map<std::string, GameMap>& getMaps() { return maps; }
    const std::map<std::string, GameMap>& getMaps() const { return maps; }
    std::map<std::string, GameObject>& getNpcs() { return npcTemplates; }
    std::map<std::string, GameObject>& getItems() { return items; }
    std::map<std::string, int>& getVariables() { return variables; }
    const std::map<std::string, int>& getVariables() const { return variables; }
    
    InputHandler inputHandler{*this};  ///< 输入处理器(绑定当前引擎实例)
    
    const std::list<GameObject>& getInventory() const { return inventoryManager.getItems(); }
        
    // 物品操作
    /**
     * @brief 使用指定物品
     * @param item 要使用的物品实例
     *
     * 执行流程：
     * 1. 查找物品定义
     * 2. 顺序执行所有使用效果
     * 3. 处理消耗品数量
     */
    void useItem(const GameObject& item);
    
    /**
     * @brief 丢弃物品到当前位置
     * @param item 要丢弃的物品实例
     *
     * 注意：
     * - 堆叠物品会自动拆分
     * - 会保留物品实例ID
     */
    void discardItem(const GameObject& item);
    
    /**
     * @brief 切换当前地图
     * @param map 目标地图名称
     */
    void setCurrentMap(const std::string& map) { currentMap = map; }
    
    /**
     * @brief 获取指定位置的对象
     * @param x 地图X坐标
     * @param y 地图Y坐标
     * @return 该位置的游戏对象(可能为空对象)
     */
    GameObject getObjectAt(int x, int y);

private:
    // 初始化方法
    /**
     * @brief 处理init代码块
     * @param fs 输入文件流
     * @param lineNumber 当前行号(引用传递会修改)
     *
     * 语法格式：
     * init {
     *   // 初始化命令
     * }
     */
    void processInitBlock(std::ifstream& fs, int& lineNumber);
    
    /**
     * @brief 处理条件代码块
     * @param fs 输入文件流
     * @param condition 条件表达式
     * @param currentLine 当前行号
     *
     * 语法格式：
     * if 条件 {
     *   // 条件成立时执行的命令
     * }
     */
    void processIfBlock(std::ifstream& fs, const std::string& condition, int currentLine);
    
    /**
     * @brief 处理物品效果块
     * @param fs 输入文件流
     * @param headerLine 块声明行
     * @param lineNumber 当前行号(引用传递会修改)
     *
     * 语法格式：
     * item 使用效果 物品名: {
     *   // 使用效果命令
     * }
     */
    void processItemEffectBlock(std::ifstream& fs, const std::string& headerLine, int& lineNumber);

    // 辅助方法
    /**
     * @brief 方向向量转字符
     * @param dx X方向变化量(-1/0/1)
     * @param dy Y方向变化量(-1/0/1)
     * @return 方向字符(u/d/l/r)
     */
    static char dirToChar(int dx, int dy);
    
    friend class SaveLoadManager;     ///< 允许存档管理器访问私有数据
    friend class ConditionEvaluator; ///< 允许条件评估器访问私有数据
};