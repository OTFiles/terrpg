// File: src/GameEngine/Rendering.cpp
#include "GameEngine.h"
#include <ncurses.h>

void GameEngine::draw() {
    clear();
    GameMap& curMap = maps[currentMap];
    
    for (int y = 0; y < viewportH; ++y) {
        for (int x = 0; x < viewportW; ++x) {
            int mapX = viewportX + x;
            int mapY = viewportY + y;
            if (mapX >= 0 && mapX < curMap.getWidth() && 
                mapY >= 0 && mapY < curMap.getHeight()) {
                GameObject obj = curMap.getObject(mapX, mapY);
                if (obj.display != ' ') mvaddch(y, x, obj.display);
            }
        }
    }

    int playerScreenX = playerX - viewportX;
    int playerScreenY = playerY - viewportY;
    char playerChar = '>';
    switch(playerDir) {
        case 'u': playerChar = '^'; break;
        case 'd': playerChar = 'v'; break;
        case 'l': playerChar = '<'; break;
        case 'r': playerChar = '>'; break;
    }
    if (playerScreenX >= 0 && playerScreenX < viewportW &&
        playerScreenY >= 0 && playerScreenY < viewportH)
        mvaddch(playerScreenY, playerScreenX, playerChar);

    drawUI();
    refresh();
}

void GameEngine::drawUI() {
    if (currentDialog) {
        int startY = viewportH + 1;
        attron(A_REVERSE);
        mvprintw(startY, 0, "【%s】:", currentDialog->speaker.c_str());
        for (size_t i = 0; i < currentDialog->lines.size(); ++i)
            mvprintw(startY + 1 + i, 0, "%s", currentDialog->lines[i].c_str());
        attroff(A_REVERSE);
    }

    if (gameState == GameState::INVENTORY) {
        int startX = viewportW + 2;
        attron(A_BOLD);
        mvprintw(0, startX, "=== 物品栏 ===");
        int idx = 0;
        for (const auto& item : inventory) {
            if (idx == selectedInventoryIndex) {
                attron(A_REVERSE);
                mvprintw(idx + 1, startX, "> %s", item.c_str());
                attroff(A_REVERSE);
            } else mvprintw(idx + 1, startX, "  %s", item.c_str());
            idx++;
        }
        attroff(A_BOLD);
    }
}