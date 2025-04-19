// include/terrpg/commands/Command.h
#pragma once
#include "terrpg/common/Exceptions.h"
#include <vector>
#include <string>
#include <memory>

namespace terrpg {

// 前置声明减少依赖
namespace core {
class GameEngine;
}

namespace commands {

class Command {
public:
    virtual ~Command() = default;
    
    /**
     * 执行命令的核心接口
     * @param engine 游戏引擎引用
     * @param tokens 命令参数列表
     * @throws CommandException 当参数验证失败时抛出
     */
    virtual void execute(core::GameEngine& engine, 
                       const std::vector<std::string>& tokens) = 0;

protected:
    /**
     * 参数验证工具方法
     * @param tokens 参数列表
     * @param minParams 最小参数数量
     * @param usage 命令用法提示
     */
    void validateParameters(const std::vector<std::string>& tokens,
                          size_t minParams,
                          const std::string& usage) const
    {
        if (tokens.size() < minParams) {
            throw common::CommandException(
                "Command requires at least " + std::to_string(minParams) + 
                " parameters\nUsage: " + usage
            );
        }
    }
};

/// 命令智能指针类型
using CommandPtr = std::unique_ptr<Command>;

} // namespace commands
} // namespace terrpg