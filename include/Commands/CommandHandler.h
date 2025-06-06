// include/Commands/CommandHandler.h

#pragma once
#include "GameEngine.h"
#include "Log.h"
#include <vector>
#include <string>

class CommandHandler {
public:
    virtual ~CommandHandler() = default;
    virtual void handle(const std::vector<std::string>& args, GameEngine& engine) = 0;
};