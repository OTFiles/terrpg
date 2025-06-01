// File: src/GameEngine/Commands/ConcreteCommands/ScoreboardCommand.cpp
#include "ScoreboardCommand.h"
#include "GameEngine.h"

void ScoreboardCommand::handle(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 2) {
        throw std::runtime_error("Usage: /scoreboard <add|set|operation>");
    }

    const std::string& subcmd = args[1];
    if (subcmd == "add") {
        handleAdd(args, engine);
    } else if (subcmd == "set") {
        handleSet(args, engine);
    } else if (subcmd == "operation") {
        handleOperation(args, engine);
    } else {
        throw std::runtime_error("未知子命令: " + subcmd);
    }
}

void ScoreboardCommand::handleAdd(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 3) {
        throw std::runtime_error("Usage: /scoreboard add <variable>");
    }
    engine.getVariables()[args[2]] = 0;
    engine.getDialogSystem().showDialog({
        {"已创建变量: " + args[2]},
        "计分板"
    });
}

void ScoreboardCommand::handleSet(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 4) {
        throw std::runtime_error("Usage: /scoreboard set <variable> <value>");
    }
    
    std::string expr;
    for (size_t i = 3; i < args.size(); ++i) {
        if (i > 3) expr += " ";
        expr += args[i];
    }
    
    try {
        int value = ConditionEvaluator::evaluateExpression(engine, expr);
        engine.getVariables()[args[2]] = value;
        engine.getDialogSystem().showDialog({
            {args[2] + " = " + std::to_string(value)},
            "计分板"
        });
    } catch (const std::exception& e) {
        throw std::runtime_error("表达式计算失败: " + std::string(e.what()));
    }
}

void ScoreboardCommand::handleOperation(const std::vector<std::string>& args, GameEngine& engine) {
    if (args.size() < 5) {
        throw std::runtime_error("Usage: /scoreboard operation <variable> <op> <expression>");
    }

    const std::string& varName = args[2];
    const std::string& op = args[3];
    
    // 构建表达式
    std::string exprStr;
    for (size_t i = 4; i < args.size(); ++i) {
        if (i > 4) exprStr += " ";
        exprStr += args[i];
    }

    try {
        int value = ConditionEvaluator::evaluateExpression(engine, exprStr);
        auto& variables = engine.getVariables();
        
        if (variables.find(varName) == variables.end()) {
            throw std::runtime_error("未定义的变量: " + varName);
        }

        if (op == "=") {
            variables[varName] = value;
        } else if (op == "+=") {
            variables[varName] += value;
        } else if (op == "-=") {
            variables[varName] -= value;
        } else if (op == "*=") {
            variables[varName] *= value;
        } else if (op == "/=") {
            if (value == 0) throw std::runtime_error("除数不能为零");
            variables[varName] /= value;
        } else {
            throw std::runtime_error("未知操作符: " + op);
        }

        engine.getDialogSystem().showDialog({
            {varName + " " + op + " " + std::to_string(value) + " → " + std::to_string(variables[varName])},
            "计分板"
        });
    } catch (const std::exception& e) {
        throw std::runtime_error("操作执行失败: " + std::string(e.what()));
    }
}