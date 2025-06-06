// src/InputHandler.cpp
#include "InputHandler.h"
#include "GameEngine.h"
#include "Log.h"
#include <ncurses.h>
#include <cstdlib>

void InputHandler::processInput(int key) {
#ifdef DEBUG
    Log log("debug.log");
#endif
    switch (static_cast<int>(engine.getGameState())) {
        case static_cast<int>(GameState::EXPLORING):
            handleExploring(key);
#ifdef DEBUG
            log.debug("EXPLORING(探索)状态");
#endif
            break;
        case static_cast<int>(GameState::INVENTORY):
            handleInventory(key);
#ifdef DEBUG
            log.debug("INVENTORY(背包)状态");
#endif
            break;
        case static_cast<int>(GameState::ITEM_OPTION):
            handleItemOption(key);
#ifdef DEBUG
            log.debug("ITEM_OPTION(物品操作)状态");
#endif
            break;
        case static_cast<int>(GameState::DIALOG):
            handleDialog(key);
#ifdef DEBUG
            log.debug("DIALOG(对话)状态");
#endif
            break;
    }
#ifdef DEBUG
        Log log("debug.log");
        log.debug("键入 ", key);
#endif
}

void InputHandler::handleExploring(int key) {
    int dx = 0, dy = 0;
    
    switch(key) {
        case KEY_UP:    dy = -1; engine.getPlayerDir() = 'u'; break;
        case KEY_DOWN:  dy = 1;  engine.getPlayerDir() = 'd'; break;
        case KEY_LEFT:  dx = -1; engine.getPlayerDir() = 'l'; break;
        case KEY_RIGHT: dx = 1;  engine.getPlayerDir() = 'r'; break;
        case 'g':
            engine.pickupItem(engine.getPlayerX(), engine.getPlayerY());
            break;
        case 'u':
            engine.tryTalkToNPC();
            break;
        case 'i':
            if (!engine.getInventoryManager().getItems().empty()) {
                engine.getGameState() = GameState::INVENTORY;
                engine.getInventoryManager().setSelectedIndex(0);
            } else {
                engine.getDialogSystem().showDialog({{"物品栏为空"}, "系统"}, engine);
            }
            break;
        case 'q':
            endwin();
            exit(0);
    }
    
    if (dx != 0 || dy != 0) {
        handleMovement(dx, dy);
    }
}

void InputHandler::handleMovement(int dx, int dy) {
    int newX = engine.getPlayerX() + dx;
    int newY = engine.getPlayerY() + dy;

    if (engine.getCurrentMap().isWalkable(newX, newY)) {
        engine.setPlayerX(newX);
        engine.setPlayerY(newY);
        engine.updateViewport();
    }
}

void InputHandler::handleInventory(int key) {
    switch(key) {
        case KEY_UP:
            if (engine.getInventoryManager().getSelectedIndex() > 0) {
                engine.getInventoryManager().setSelectedIndex(engine.getInventoryManager().getSelectedIndex() - 1);
            }
            break;
        case KEY_DOWN:
            if (engine.getInventoryManager().getSelectedIndex() < static_cast<int>(engine.getInventoryManager().getItems().size())-1) {
                engine.getInventoryManager().setSelectedIndex(engine.getInventoryManager().getSelectedIndex() + 1);
            }
            break;
        case '\n': {
            auto& items = engine.getInventoryManager().getItems();
            int selectedIdx = engine.getInventoryManager().getSelectedIndex();
            auto it = items.begin();
            std::advance(it, selectedIdx);

            Dialog dialog;
            dialog.lines = {"使用", "丢弃"};
            dialog.speaker = it->name;
            dialog.selectedOption = 0;
            engine.getDialogSystem().showDialog(dialog, engine);

            engine.setGameState(GameState::ITEM_OPTION);
            break;
        }
        case 27: // ESC
            engine.setGameState(GameState::EXPLORING);
            break;
    }
}

void InputHandler::handleItemOption(int key) {
    switch(key) {
        case KEY_UP:
        case KEY_DOWN: {
            auto& dialog = engine.getDialogSystem().getCurrentDialog();
            if (dialog && !dialog->lines.empty()) {
                std::rotate(
                    dialog->lines.begin() + dialog->selectedOption,
                    dialog->lines.begin() + 1, 
                    dialog->lines.end()
                );
            }
            break;
        }
        case '\n': {
            auto& items = engine.getInventoryManager().getItems();
            int selectedIdx = engine.getInventoryManager().getSelectedIndex();
            auto it = items.begin();
            std::advance(it, selectedIdx);

            if (engine.getDialogSystem().getCurrentDialog()->lines[0] == "使用") {
                engine.useItem(*it);
            } else {
                engine.discardItem(*it);
            }
            engine.setGameState(GameState::EXPLORING);
            break;
        }
        case 27: { // ESC
            engine.setGameState(GameState::INVENTORY);
            break;
        }
    }
}

void InputHandler::handleDialog(int key) {
    if (key != ERR) {
        engine.getDialogSystem().closeDialog(engine);
        engine.setGameState(GameState::EXPLORING);
    }
}