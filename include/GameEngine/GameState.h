// include/GameEngine/GameState.h
#pragma once
#include <string>
#include <vector>

/**
 * @enum GameState
 * @brief 游戏状态枚举，表示游戏当前所处的不同模式
 * 
 * 状态说明：
 * - EXPLORING: 探索模式 - 玩家在地图上移动和互动
 * - INVENTORY: 背包模式 - 玩家查看和管理背包物品
 * - ITEM_OPTION: 物品操作模式 - 玩家对选定物品执行操作
 * - DIALOG: 对话模式 - 玩家与NPC对话或查看系统消息
 */
enum class GameState {
    EXPLORING,    ///< 探索模式（默认状态）
    INVENTORY,    ///< 背包查看模式
    ITEM_OPTION,  ///< 物品选项操作模式
    DIALOG        ///< 对话显示模式
};

/**
 * @struct Dialog
 * @brief 对话数据结构，存储对话内容和相关信息
 * 
 * 用于在游戏对话系统中显示单行或多选对话内容
 */
struct Dialog {
    std::vector<std::string> lines;  ///< 对话内容行（多选对话时包含选项）
    std::string speaker;             ///< 对话发起者名称（如NPC名或"系统"）
    int selectedOption = 0;          ///< 当前选中的选项索引（多选对话时使用）
    
    /**
     * @brief 默认构造函数
     */
    Dialog() = default;
    
    /**
     * @brief 带参数构造函数
     * @param l 对话内容行向量
     * @param s 对话发起者名称
     * 
     * 使用示例：
     * Dialog dlg({"选项1", "选项2"}, "商人");
     */
    Dialog(std::vector<std::string> l, std::string s) 
        : lines(std::move(l)), speaker(std::move(s)) {}
};