// include/GameEngine/Renderer.h
#pragma once
#include "GameMap.h"
#include "GameObject.h"
#include "DialogSystem.h"
#include <list>
#include <vector>
#include <ncurses.h>
#include <cmath>
#include <memory>

class GameEngine;

/**
 * @class Renderer
 * @brief 游戏渲染器，负责所有游戏画面的绘制和显示
 * 
 * 功能特点：
 * - 基于ncurses的终端渲染
 * - 支持视口计算和地图显示
 * - 处理玩家角色、UI和对话框的渲染
 * - 自适应终端尺寸变化
 */
class Renderer {
    int termWidth;      ///< 终端窗口宽度（字符数）
    int termHeight;     ///< 终端窗口高度（字符数）
    
    // 视口相关参数
    int viewportX;      ///< 视口左上角在地图上的X坐标
    int viewportY;      ///< 视口左上角在地图上的Y坐标
    int viewportW;      ///< 视口宽度（字符数）
    int viewportH;      ///< 视口高度（字符数）
    
    // 颜色对定义
    const int COLOR_PAIR_DEFAULT = 1;    ///< 默认颜色对（白底黑字）
    const int COLOR_PAIR_HIGHLIGHT = 2;  ///< 高亮颜色对（黑底白字）
    
#ifdef DEBUG
    std::string debugMessage; ///< 调试信息
#endif
    
public:
    /**
     * @brief 构造函数
     * 
     * 初始化ncurses屏幕和颜色设置
     */
    Renderer();
    
    /**
     * @brief 析构函数
     * 
     * 清理ncurses资源
     */
    ~Renderer();
    
    /**
     * @brief 初始化ncurses屏幕
     */
    void initScreen();
    
    /**
     * @brief 计算当前视口参数
     * @param engine 游戏引擎引用
     * 
     * 根据玩家位置和终端尺寸计算：
     * - 视口大小（不超过地图边界）
     * - 视口位置（玩家居中）
     */
    void calculateViewport(const GameEngine& engine);
    
    /**
     * @brief 主渲染函数
     * @param engine 游戏引擎引用
     * 
     * 渲染流程：
     * 1. 清屏
     * 2. 计算视口
     * 3. 绘制地图和玩家
     * 4. 绘制UI元素
     * 5. 刷新屏幕
     */
    void render(const GameEngine& engine);
    
#ifdef DEBUG
    /**
     * @brief 设置调试信息
     * @param message 要显示的调试信息
     */
    void setDebugMessage(const std::string& message);
#endif
    
private:
    // ================= 地图渲染 =================
    
    /**
     * @brief 绘制地图区域
     * @param engine 游戏引擎引用
     * 
     * 包括：
     * - 地图边框
     * - 地图内容
     * - 玩家角色
     */
    void drawMap(const GameEngine& engine);
    
    /**
     * @brief 绘制地图边框
     * @param startX 边框起始X坐标（屏幕坐标）
     * @param startY 边框起始Y坐标（屏幕坐标）
     * 
     * 使用Unicode制表符绘制美观的边框
     */
    void drawBorder(int startX, int startY);
    
    /**
     * @brief 绘制玩家角色
     * @param engine 游戏引擎引用
     * @param mapStartX 地图起始X坐标（屏幕坐标）
     * @param mapStartY 地图起始Y坐标（屏幕坐标）
     * 
     * 根据玩家朝向显示不同字符：
     * - 上: ^
     * - 下: v
     * - 左: <
     * - 右: >
     */
    void drawPlayer(const GameEngine& engine, int mapStartX, int mapStartY);
    
    /**
     * @brief 绘制地图内容
     * @param engine 游戏引擎引用
     * @param mapStartX 地图起始X坐标（屏幕坐标）
     * @param mapStartY 地图起始Y坐标（屏幕坐标）
     * 
     * 只渲染视口范围内的地图对象
     */
    void drawMapContent(const GameEngine& engine, int mapStartX, int mapStartY);
    
    // ================= UI渲染 =================
    
    /**
     * @brief 绘制UI元素
     * @param engine 游戏引擎引用
     * 
     * 根据游戏状态绘制：
     * - 对话状态: 对话框
     * - 背包状态: 物品栏
     */
    void drawUI(const GameEngine& engine);
    
    /**
     * @brief 绘制对话框
     * @param dialog 要渲染的对话数据
     * 
     * 包括：
     * - 带标题的边框
     * - 对话内容
     * - 多选项的高亮显示
     */
    void drawDialog(const Dialog& dialog);
    
    /**
     * @brief 绘制物品栏
     * @param inventory 物品列表
     * @param selectedIndex 当前选中物品索引
     * 
     * 显示规则：
     * - 可堆叠物品显示数量
     * - 选中物品高亮显示
     * - 自动处理长列表滚动
     */
    void drawInventory(const std::list<GameObject>& inventory, int selectedIndex);
    
    /**
     * @brief 获取边界字符（用于地图边缘）
     * @param mapX 地图X坐标
     * @param mapY 地图Y坐标
     * @param map 游戏地图引用
     * @return 合适的边界字符
     */
    wchar_t getBorderChar(int mapX, int mapY, const GameMap& map);
    
#ifdef DEBUG
    /**
     * @brief 绘制调试信息（在屏幕底部）
     */
    void drawDebugInfo();
#endif
};