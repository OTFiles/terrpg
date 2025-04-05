// src/core/GameState.h
#pragma once

enum class GameState { 
    EXPLORING,   // 探索状态
    INVENTORY,   // 物品栏状态
    ITEM_OPTION, // 物品操作选项
    DIALOG       // 对话框状态
};