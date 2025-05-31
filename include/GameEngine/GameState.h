// include/GameState.h
#pragma once
#include <string>
#include <vector>

enum class GameState {
    EXPLORING,
    INVENTORY,
    ITEM_OPTION,
    DIALOG
};

struct Dialog {
    std::vector<std::string> lines;
    std::string speaker;
    int selectedOption = 0;
    Dialog() = default;
    Dialog(std::vector<std::string> l, std::string s) 
        : lines(std::move(l)), speaker(std::move(s)) {}
};