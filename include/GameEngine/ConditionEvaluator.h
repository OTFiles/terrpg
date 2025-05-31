// include/ConditionEvaluator.h
#pragma once
#include <vector>
#include <string>

class GameEngine;

class ConditionEvaluator {
public:
    static bool evaluate(GameEngine& engine, const std::string& condition);
    static int evaluateExpression(GameEngine& engine, const std::string& expr);

private:
    static std::vector<std::string> tokenize(const std::string& line);
    static std::string replaceVariables(GameEngine& engine, const std::string& expr);
};