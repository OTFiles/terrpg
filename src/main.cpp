// File: src/main.cpp
#include "GameEngine.h"
#include <iostream>
#include <exception>

int main() {
    try {
        // 初始化游戏引擎
        GameEngine engine;
        
        // 加载游戏数据
        engine.loadGame("game.txt");
        
        // 启动主游戏循环
        engine.startGameLoop();
        
        // 游戏正常退出时保存数据
        engine.saveGame("autosave.sav");
    }
    catch (const std::exception& e) {
        // 异常处理：恢复终端正常模式并显示错误信息
        endwin();
        std::cerr << "\n发生严重错误: " << e.what() << std::endl;
        std::cerr << "请检查游戏文件是否完整，或联系开发者\n";
        return EXIT_FAILURE;
    }
    catch (...) {
        endwin();
        std::cerr << "\n发生未知错误" << std::endl;
        return EXIT_FAILURE;
    }
    
    // 正常退出
    return EXIT_SUCCESS;
}