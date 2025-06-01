// include/DialogSystem.h
#pragma once
#include "GameState.h"
#include <optional>

class GameEngine;

/**
 * @class DialogSystem
 * @brief 游戏对话系统，负责管理和渲染游戏中的所有对话界面
 * 
 * 主要功能包括：
 * - 显示单行或多选对话内容
 * - 处理玩家在对话中的输入选择
 * - 自动检测玩家附近的NPC并触发对话
 * - 支持条件对话（根据游戏状态显示不同对话）
 */
class DialogSystem {
private:
    mutable std::optional<Dialog> currentDialog; ///< 当前活动的对话（可选值，无对话时为empty）
    
public:
    /**
     * @brief 显示对话内容
     * @param dialog 要显示的对话对象
     * 
     * 功能说明：
     * - 自动处理文本换行（每行最多40字符）
     * - 支持多行对话内容
     * - 设置当前活动对话
     */
    void showDialog(Dialog dialog) const;
    
    /**
     * @brief 获取当前对话（只读）
     * @return 当前对话的可选引用
     */
    const std::optional<Dialog>& getCurrentDialog() const { return currentDialog; }
    
    /**
     * @brief 处理对话输入
     * @param engine 游戏引擎引用
     * @param key 玩家按下的键值
     * 
     * 输入处理逻辑：
     * - 方向键上下：切换多选对话中的选项
     * - 回车键：确认选择并执行关联命令
     * - 选择后自动退出对话状态
     */
    void handleInput(GameEngine& engine, int key);
    
    /**
     * @brief 尝试与NPC对话
     * @param engine 游戏引擎引用
     * 
     * NPC检测规则：
     * 1. 检查玩家四方向相邻位置是否存在NPC
     * 2. 优先显示满足条件的特殊对话
     * 3. 若无特殊对话则显示默认对话
     * 4. 附近无NPC时显示系统提示
     */
    void tryTalkToNPC(GameEngine& engine);
    
    /**
     * @brief 更新对话显示
     * @param engine 游戏引擎引用
     * 
     * 渲染规则：
     * - 在视口下方显示对话区域
     * - 高亮显示当前选中的选项
     * - 显示对话发起者名称
     */
    void updateDialogDisplay(const GameEngine& engine);
    
    /**
     * @brief 获取当前对话（可修改）
     * @return 当前对话的可选引用
     */
    std::optional<Dialog>& getCurrentDialog() { return currentDialog; }
    
    /**
     * @brief 重置对话系统
     * 
     * 清除当前对话内容，退出对话状态
     */
    void resetDialog() { currentDialog.reset(); }
};