// include/terrpg/core/scripting/ScriptBlock.h

#pragma once
#include "../../common/Types.h"
#include "../../common/Exceptions.h"
#include "Condition.h"
#include "Expression.h"
#include <vector>
#include <memory>

namespace terrpg::core::scripting {

class Command;

class ScriptBlock {
public:
    enum class BlockType {
        IF,
        WHILE,
        COMMAND_SEQUENCE,
        FUNCTION
    };

    struct ExecutionContext {
        Map<String, int>& variables;
        const Set<String>& inventory;
        const Set<String>& visitedMarkers;
    };

    // 解析脚本块
    void parse(const Vector<String>& lines, int baseLineNumber);

    // 执行脚本块
    bool execute(ExecutionContext& context) const;

    // 块元信息
    BlockType getType() const noexcept { return type_; }
    int getStartLine() const noexcept { return startLine_; }
    int getEndLine() const noexcept { return endLine_; }

private:
    void parseConditionBlock(const Vector<String>& lines, size_t& linePtr);
    void parseCommandSequence(const Vector<String>& lines, size_t& linePtr);

    BlockType type_ = BlockType::COMMAND_SEQUENCE;
    int startLine_ = 0;
    int endLine_ = 0;
    Condition condition_;
    Vector<std::unique_ptr<Command>> commands_;
    Vector<std::unique_ptr<ScriptBlock>> childBlocks_;
};

} // namespace terrpg::core::scripting