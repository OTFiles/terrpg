// src/core/scripting/ScriptBlock.cpp

#include "ScriptBlock.h"
#include "Parser.h"
#include "../commands/CommandFactory.h"
#include <stdexcept>

namespace terrpg::core::scripting {

void ScriptBlock::parse(const Vector<String>& lines, int baseLineNumber) {
    if(lines.empty()) return;

    startLine_ = baseLineNumber;
    endLine_ = baseLineNumber + lines.size() - 1;

    size_t linePtr = 0;
    const String& firstLine = lines[0];

    if(Parser::isControlStructure(firstLine)) {
        type_ = Parser::determineBlockType(firstLine);
        parseConditionBlock(lines, linePtr);
    } else {
        parseCommandSequence(lines, linePtr);
    }
}

bool ScriptBlock::execute(ExecutionContext& context) const {
    switch(type_) {
        case BlockType::IF:
            if(condition_.evaluate(context)) {
                for(const auto& cmd : commands_) {
                    if(!cmd->execute(context)) return false;
                }
            }
            break;
            
        case BlockType::WHILE:
            while(condition_.evaluate(context)) {
                for(const auto& cmd : commands_) {
                    if(!cmd->execute(context)) return false;
                }
            }
            break;
            
        default:
            for(const auto& cmd : commands_) {
                if(!cmd->execute(context)) return false;
            }
    }
    return true;
}

void ScriptBlock::parseConditionBlock(const Vector<String>& lines, size_t& linePtr) {
    // 解析首行条件
    condition_ = Parser::parseCondition(lines[linePtr++]);
    
    // 解析命令序列
    Vector<String> commandLines;
    while(linePtr < lines.size()) {
        if(lines[linePtr] == "end") {
            linePtr++;
            break;
        }
        commandLines.push_back(lines[linePtr++]);
    }
    parseCommandSequence(commandLines, linePtr);
}

void ScriptBlock::parseCommandSequence(const Vector<String>& lines, size_t& linePtr) {
    CommandFactory& factory = CommandFactory::getInstance();
    
    for(const auto& line : lines) {
        auto tokens = StringUtils::split(line, ' ');
        if(tokens.empty()) continue;

        try {
            auto cmd = factory.createCommand(tokens[0]);
            cmd->parse(tokens);
            commands_.push_back(std::move(cmd));
        } catch(const common::CommandException& e) {
            throw common::ScriptException(
                "Failed to parse command: " + line,
                startLine_ + linePtr
            );
        }
        linePtr++;
    }
}

} // namespace terrpg::core::scripting