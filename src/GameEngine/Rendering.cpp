// File: src/GameEngine/Rendering.cpp
#include "GameEngine.h"
#include <ncurses.h>
#include <cmath>

// 宽字符版边框绘制辅助函数
wchar_t GameEngine::getBorderChar(int mapX, int mapY, const GameMap& map) {
    const bool isTopEdge = (mapY == 0);
    const bool isBottomEdge = (mapY == map.getHeight()-1);
    const bool isLeftEdge = (mapX == 0);
    const bool isRightEdge = (mapX == map.getWidth()-1);

    if (isTopEdge && isLeftEdge)    return L'╭';
    if (isTopEdge && isRightEdge)   return L'╮';
    if (isBottomEdge && isLeftEdge) return L'╰';
    if (isBottomEdge && isRightEdge)return L'╯';
    if (isTopEdge || isBottomEdge)  return L'─';
    if (isLeftEdge || isRightEdge)  return L'│';
    return L' ';
}

void GameEngine::draw() {
    // 获取终端尺寸
    getmaxyx(stdscr, termHeight, termWidth);
    viewportW = termWidth - 24; // 右侧留出物品栏空间
    viewportH = termHeight - 5; // 下方留出对话框空间

    updateViewport(); // 根据新尺寸更新视口位置

    clear();
    GameMap& curMap = maps[currentMap];

    // 绘制地图边框
    const int mapStartX = std::max(0, (termWidth - curMap.getWidth()*2)/2);
    const int mapStartY = std::max(0, (termHeight - curMap.getHeight())/2);
    
    // 上边框
    mvaddwstr(mapStartY-1, mapStartX-1, L"╭");
    for(int x=0; x<curMap.getWidth(); x++) {
        mvaddwstr(mapStartY-1, mapStartX+x, L"─");
    }
    mvaddwstr(mapStartY-1, mapStartX+curMap.getWidth(), L"╮");

    // 侧边框
    for(int y=0; y<curMap.getHeight(); y++) {
        mvaddwstr(mapStartY+y, mapStartX-1, L"│");
        mvaddwstr(mapStartY+y, mapStartX+curMap.getWidth(), L"│");
    }

    // 下边框
    mvaddwstr(mapStartY+curMap.getHeight(), mapStartX-1, L"╰");
    for(int x=0; x<curMap.getWidth(); x++) {
        mvaddwstr(mapStartY+curMap.getHeight(), mapStartX+x, L"─");
    }
    mvaddwstr(mapStartY+curMap.getHeight(), mapStartX+curMap.getWidth(), L"╯");

    // 绘制地图内容
    for (int y = 0; y < viewportH; ++y) {
        for (int x = 0; x < viewportW; ++x) {
            int mapX = viewportX + x;
            int mapY = viewportY + y;

            if (mapX >= 0 && mapX < curMap.getWidth() && 
                mapY >= 0 && mapY < curMap.getHeight()) {
                GameObject obj = curMap.getObject(mapX, mapY);
                mvaddch(mapStartY + (mapY - viewportY), mapStartX + (mapX - viewportX), obj.display != ' ' ? obj.display : getBorderChar(mapX, mapY, curMap));
            }
        }
    }

    // 绘制玩家角色
    wchar_t playerChar = L'>';
    switch(playerDir) {
        case 'u': playerChar = L'^'; break;
        case 'd': playerChar = L'v'; break;
        case 'l': playerChar = L'<'; break;
        case 'r': playerChar = L'>'; break;
    }
    mvwaddwstr(stdscr, 
               mapStartY + (playerY - viewportY), 
               mapStartX + (playerX - viewportX), 
               &playerChar);

    this->drawUI();
    refresh();
}

void GameEngine::drawUI() {
    const int uiStartX = viewportW + 2;

    if (currentDialog) {
        int startY = viewportH + 1;
        attron(A_REVERSE);
        mvprintw(startY, 0, "【%s】:", currentDialog->speaker.c_str());
        for (size_t i = 0; i < currentDialog->lines.size(); ++i)
            mvprintw(startY + 1 + i, 0, "%s", currentDialog->lines[i].c_str());
        attroff(A_REVERSE);
    }

    if (gameState == GameState::INVENTORY) {
        attron(A_BOLD);
        mvprintw(0, uiStartX, "=== 物品栏 ===");
        int idx = 0;
        for (const auto& item : inventory) {
            // 获取显示名称和数量
            std::string displayName = item.name;
            const bool stackable = item.getProperty<int>("stackable", 1); // 默认可堆叠
            const int count = item.getProperty<int>("count", 1);

            if (stackable && count > 1) {
                displayName += " x" + std::to_string(count);
            }

            if (idx == selectedInventoryIndex) {
                attron(A_REVERSE);
                mvprintw(idx + 1, uiStartX, "> %s", displayName.c_str());
                attroff(A_REVERSE);
            } else {
                mvprintw(idx + 1, uiStartX, "  %s", displayName.c_str());
            }
            idx++;
        }
        attroff(A_BOLD);
    }
}