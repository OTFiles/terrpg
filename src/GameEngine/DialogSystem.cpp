// src/GameEngine/DialogSystem.cpp
#include "DialogSystem.h"
#include "GameEngine.h"
#include "Log.h"
#include <ncurses.h>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

void DialogSystem::showDialog(Dialog dialog) const {
#ifdef DEBUG
    Log log("debug.log");
    // 遍历Dialog对象的lines
    for (const auto& line : dialog.lines) {
        // 使用log.debug输出speaker和当前行内容
        log.debug("调用 showDialog", dialog.speaker, ":", line);
    }
#endif
    // 自动换行处理
    vector<string> wrappedLines;
    const int maxWidth = 40; // 根据视口宽度调整
    
    for (const auto& line : dialog.lines) {
        stringstream ss(line);
        string segment;
        vector<string> tempLines;
        
        while (getline(ss, segment, '\n')) {
            size_t pos = 0;
            while (pos < segment.length()) {
                size_t end = min(pos + maxWidth, segment.length());
                if (end != segment.length()) {
                    size_t spacePos = segment.rfind(' ', end);
                    if (spacePos != string::npos && spacePos > pos) {
                        end = spacePos;
                    }
                }
                wrappedLines.push_back(segment.substr(pos, end - pos));
                pos = end + (end != segment.length() ? 1 : 0);
            }
        }
    }
    
    dialog.lines = wrappedLines;
    currentDialog = dialog;
}

void DialogSystem::handleInput(GameEngine& engine, int key) {
    if (!currentDialog) return;

    // 处理选项切换
    if (currentDialog->lines.size() > 1) {
        if (key == KEY_UP) {
            currentDialog->selectedOption = 
                (currentDialog->selectedOption - 1 + currentDialog->lines.size()) % 
                currentDialog->lines.size();
        }
        else if (key == KEY_DOWN) {
            currentDialog->selectedOption = 
                (currentDialog->selectedOption + 1) % currentDialog->lines.size();
        }
    }

    // 确认选择
    if (key == '\n') {
        // 触发后续命令逻辑
        if (currentDialog->lines.size() > 1) {
            string selected = currentDialog->lines[currentDialog->selectedOption];
            vector<string> tokens = engine.tokenize(selected);
            engine.runCommand(tokens);
        }
        closeDialog(engine);
    }
}

void DialogSystem::tryTalkToNPC(GameEngine& engine) {
    const vector<pair<int, int>> directions = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}
    };
    
    for (const auto& [dx, dy] : directions) {
        GameObject obj = engine.getCurrentMap().getObject(
            engine.getPlayerX() + dx, 
            engine.getPlayerY() + dy
        );
        
        if (obj.isType("npc") && !obj.dialogues.empty()) {
            // 检查条件对话
            for (const auto& [cond, dialog] : obj.dialogues) {
                if (cond != "default" && engine.evalCondition(cond)) {
                    showDialog({engine.tokenize(dialog), obj.name});
                    engine.setGameState(GameState::DIALOG);
                    return;
                }
            }
            
            // 默认对话
            if (obj.dialogues.count("default")) {
                showDialog({engine.tokenize(obj.dialogues.at("default")), obj.name});
                engine.setGameState(GameState::DIALOG);
                return;
            }
        }
    }
    
    engine.getDialogSystem().showDialog({
        {"附近没有可对话的NPC"},
        "系统"
    });
}

// 关闭对话框实现
void DialogSystem::closeDialog(GameEngine& engine) {
    currentDialog.reset();
    engine.setGameState(GameState::EXPLORING);
}