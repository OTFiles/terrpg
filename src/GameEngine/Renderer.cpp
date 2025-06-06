// src/Renderer.cpp
#include "Renderer.h"
#include "GameEngine.h"
#include <algorithm>
#include <sstream>
#include <string>

Renderer::Renderer() {
    initScreen();
}

Renderer::~Renderer() {
    endwin();
}

void Renderer::initScreen() {
    initscr();
    setlocale(LC_ALL, "");
    use_default_colors();
    start_color();
    init_pair(COLOR_PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_PAIR_HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

void Renderer::calculateViewport(const GameEngine& engine) {
    getmaxyx(stdscr, termHeight, termWidth);
    viewportW = std::min(termWidth - 20, engine.getCurrentMap().getWidth());
    viewportH = std::min(termHeight - 5, engine.getCurrentMap().getHeight());

    const GameMap& currentMap = engine.getCurrentMap();
    viewportX = std::max(0, std::min(engine.getPlayerX() - viewportW/2, 
                   currentMap.getWidth() - viewportW));
    viewportY = std::max(0, std::min(engine.getPlayerY() - viewportH/2, 
                   currentMap.getHeight() - viewportH));
}

// 主渲染函数
void Renderer::render(const GameEngine& engine) {
    clear();
    calculateViewport(engine);
    drawMap(engine);  // 绘制地图（包括玩家）
    drawUI(engine);  // 绘制UI
#ifdef DEBUG
    if (!debugMessage.empty()) {
        drawDebugInfo();
    }
#endif
    refresh();       // 刷新屏幕显示
}

void Renderer::drawMap(const GameEngine& engine) {
    const int mapStartX = std::max(0, (termWidth - viewportW)/2);
    const int mapStartY = std::max(0, (termHeight - viewportH)/2);
    
    // 绘制地图边框
    drawBorder(mapStartX, mapStartY);
    
    // 绘制地图内容
    drawMapContent(engine, mapStartX, mapStartY);
    
    // 绘制玩家
    drawPlayer(engine, mapStartX, mapStartY);
}

void Renderer::drawBorder(int startX, int startY) {
    // 上边框
    mvwaddwstr(stdscr, startY-1, startX-1, L"╭");
    for(int x=0; x<viewportW; x++) {
        mvwaddwstr(stdscr, startY-1, startX+x, L"─");
    }
    mvwaddwstr(stdscr, startY-1, startX+viewportW, L"╮");

    // 侧边框
    for(int y=0; y<viewportH; y++) {
        mvwaddwstr(stdscr, startY+y, startX-1, L"│");
        mvwaddwstr(stdscr, startY+y, startX+viewportW, L"│");
    }

    // 下边框
    mvwaddwstr(stdscr, startY+viewportH, startX-1, L"╰");
    for(int x=0; x<viewportW; x++) {
        mvwaddwstr(stdscr, startY+viewportH, startX+x, L"─");
    }
    mvwaddwstr(stdscr, startY+viewportH, startX+viewportW, L"╯");
}

void Renderer::drawMapContent(const GameEngine& engine, int mapStartX, int mapStartY) {
    const GameMap& currentMap = engine.getCurrentMap();
    
    // 只绘制视口内的地图内容
    for (int relY = 0; relY < viewportH; relY++) {
        for (int relX = 0; relX < viewportW; relX++) {
            int mapX = viewportX + relX;
            int mapY = viewportY + relY;

            // 确保坐标在地图范围内
            if (mapX < 0 || mapX >= currentMap.getWidth() ||
                mapY < 0 || mapY >= currentMap.getHeight()) {
                continue;
            }
            
            GameObject obj = currentMap.getObject(mapX, mapY);
            if (obj.display != L' ') {
                wchar_t wstr[2] = { obj.display, L'\0' };
                mvwaddwstr(stdscr, mapStartY + relY, mapStartX + relX, wstr);
            }
        }
    }
}

void Renderer::drawPlayer(const GameEngine& engine, int mapStartX, int mapStartY) {
    wchar_t playerChar = L' ';
    switch(engine.getPlayerDir()) {
        case 'u': playerChar = L'^'; break;
        case 'd': playerChar = L'v'; break;
        case 'l': playerChar = L'<'; break;
        case 'r': playerChar = L'>'; break;
    }
    int playerScreenX = mapStartX + (engine.getPlayerX() - viewportX);
    int playerScreenY = mapStartY + (engine.getPlayerY() - viewportY);
    
    // 高亮显示玩家
    attron(A_BOLD | COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    mvwaddch(stdscr, playerScreenY, playerScreenX, playerChar);
    attroff(A_BOLD | COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
}

void Renderer::drawUI(const GameEngine& engine) {
    if (engine.getDialogSystem().getCurrentDialog()) {
        drawDialog(engine.getDialogSystem().getCurrentDialog().value());
    }

    if (engine.getGameState() == GameState::INVENTORY) {
        drawInventory(engine.getInventory(), 
                      engine.getInventoryManager().getSelectedIndex());
    }
}

void Renderer::drawDialog(const Dialog& dialog) {
    const int dialogWidth = termWidth - 2;
    const int dialogHeight = dialog.lines.size() + 2; // 标题+内容+边框
    const int startX = 1;
    const int startY = termHeight - dialogHeight - 1;

    // 绘制对话框边框
    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvaddch(startY, startX, ACS_ULCORNER);
    mvaddch(startY, startX + dialogWidth, ACS_URCORNER);
    mvaddch(startY + dialogHeight, startX, ACS_LLCORNER);
    mvaddch(startY + dialogHeight, startX + dialogWidth, ACS_LRCORNER);
    
    for (int x = startX + 1; x < startX + dialogWidth; x++) {
        mvaddch(startY, x, ACS_HLINE);
        mvaddch(startY + dialogHeight, x, ACS_HLINE);
    }

    for (int y = startY + 1; y < startY + dialogHeight; y++) {
        mvaddch(y, startX, ACS_VLINE);
        mvaddch(y, startX + dialogWidth, ACS_VLINE);
    }
    
    // 绘制标题栏
    attron(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_BOLD);
    std::string title = "【 " + dialog.speaker + " 】";
    int titleX = startX + (dialogWidth - title.length()) / 2;
    mvprintw(startY, titleX, "%s", title.c_str());
    attroff(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_BOLD);
    
    // 绘制对话内容
    for (size_t i = 0; i < dialog.lines.size(); ++i) {
        mvprintw(startY + 2 + i, startX + 2, "%s", dialog.lines[i].c_str());
    }
}

void Renderer::drawInventory(const std::list<GameObject>& inventory, int selectedIndex) {
    const int invWidth = std::min(termWidth - 2, 20);
    const int invHeight = std::min(termHeight - 2, 25);
    const int uiStartX = termWidth - invWidth - 5;
    const int uiStartY = 1;
    
    // 绘制物品栏边框
    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    // 上边框
    mvwaddwstr(stdscr, uiStartY, uiStartX, L"╭");
    for (int x = 1; x < invWidth; x++) {
        mvwaddwstr(stdscr, uiStartY, uiStartX + x, L"─");
    }
    mvwaddwstr(stdscr, uiStartY, uiStartX + invWidth, L"╮");

    // 侧边框
    for (int y = uiStartY + 1; y < uiStartY + invHeight; y++) {
        mvwaddwstr(stdscr, y, uiStartX, L"│");
        mvwaddwstr(stdscr, y, uiStartX + invWidth, L"│");
    }
    
    // 下边框
    mvwaddwstr(stdscr, uiStartY + invHeight, uiStartX, L"╰");
    for (int x = 1; x < invWidth; x++) {
        mvwaddwstr(stdscr, uiStartY + invHeight, uiStartX + x, L"─");
    }
    mvwaddwstr(stdscr, uiStartY + invHeight, uiStartX + invWidth, L"╯");
     
    // 绘制标题（放在边框内）
    attron(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_BOLD);
    std::string title = "物品栏";
    mvwprintw(stdscr, uiStartY + 1, uiStartX + (invWidth - title.length())/2, "%s", title.c_str());
    attroff(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT) | A_BOLD);
    
    // 绘制物品列表（从边框内第一行开始）
    int idx = 0;
    for (const auto& item : inventory) {
        if (idx < invHeight - 1) { // 确保不超出边框
            std::string displayName = item.name;
            if (item.getProperty<int>("stackable", 1)) {
                int count = item.getProperty<int>("count", 1);
                if (count > 1) displayName += " x" + std::to_string(count);
            }

            int itemY = uiStartY + 2 + idx;
            if (idx == selectedIndex) {
                attron(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
                mvwprintw(stdscr, itemY, uiStartX + 2, "> %s", displayName.c_str());
                attroff(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            } else {
                mvwprintw(stdscr, itemY, uiStartX + 2, "  %s", displayName.c_str());
            }
        }
        idx++;
    }
}

#ifdef DEBUG
void Renderer::setDebugMessage(const std::string& message) {
    debugMessage = message;
}

void Renderer::drawDebugInfo() {
    // 在屏幕底部显示调试信息
    int y = termHeight - 1;
    
    // 清除行并设置背景
    attron(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    move(y, 0);
    clrtoeol();
    mvprintw(y, 0, "[DEBUG] %s", debugMessage.c_str());
    attroff(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
}
#endif