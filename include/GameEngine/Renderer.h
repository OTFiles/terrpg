// include/Renderer.h
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

class Renderer {
    int termWidth;
    int termHeight;
    
    // 视口相关参数
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    
    // 颜色对定义
    const int COLOR_PAIR_DEFAULT = 1;
    const int COLOR_PAIR_HIGHLIGHT = 2;
    
public:
    Renderer();
    ~Renderer();
    
    void initScreen();
    void calculateViewport(const GameEngine& engine);
    void render(const GameEngine& engine);
    
private:
    void drawMap(const GameEngine& engine);
    void drawUI(const GameEngine& engine);
    void drawBorder(int startX, int startY);
    void drawPlayer(const GameEngine& engine, int mapStartX, int mapStartY);
    void drawDialog(const Dialog& dialog);
    void drawInventory(const std::list<GameObject>& inventory, int selectedIndex);
    void drawMapContent(const GameEngine& engine, int mapStartX, int mapStartY);
    
    wchar_t getBorderChar(int mapX, int mapY, const GameMap& map);
};