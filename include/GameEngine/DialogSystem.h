// include/DialogSystem.h
#pragma once
#include "GameState.h"
#include <optional>

class GameEngine;

class DialogSystem {
private:
    mutable std::optional<Dialog> currentDialog;
    
public:
    void showDialog(Dialog dialog) const;
    const std::optional<Dialog>& getCurrentDialog() const { return currentDialog; }
    void handleInput(GameEngine& engine, int key);
    void tryTalkToNPC(GameEngine& engine);
    void updateDialogDisplay(const GameEngine& engine);
    std::optional<Dialog>& getCurrentDialog() { return currentDialog; }
    void resetDialog() { currentDialog.reset(); }
};