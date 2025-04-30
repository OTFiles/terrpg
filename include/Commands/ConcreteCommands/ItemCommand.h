// File: src/GameEngine/Commands/ConcreteCommands/ItemCommand.h
#pragma once
#include "../CommandHandler.h"
#include "../../GameObject.h"

class ItemCommand : public CommandHandler {
public:
    void handle(const std::vector<std::string>& args, GameEngine& engine) override;

private:
    void handleDefine(const std::vector<std::string>& args, GameEngine& engine);
    void handleSetProperty(const std::vector<std::string>& args, GameEngine& engine);
    
    // 属性类型转换器
    template<typename T>
    T parsePropertyValue(const std::string& value) {
        std::istringstream iss(value);
        T result;
        iss >> result;
        return result;
    }
};