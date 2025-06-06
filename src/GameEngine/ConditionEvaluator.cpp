// src/GameEngine/ConditionEvaluator.cpp
#include "ConditionEvaluator.h"
#include "GameEngine.h"
#include "Log.h"
#include <regex>
#include <algorithm>
#include <sstream>
#include <variant>

using namespace std;

vector<string> ConditionEvaluator::tokenize(const string& line) {
    istringstream iss(line);
    return {istream_iterator<string>{iss}, istream_iterator<string>{}};
}

string ConditionEvaluator::replaceVariables(GameEngine& engine, const string& expr) {
    regex varRegex("\\{([^}]+)\\}");
    smatch match;
    string result = expr;
    
    while(regex_search(result, match, varRegex)) {
        string varName = match[1];
        auto& variables = engine.getVariables();
        int value = variables.count(varName) ? variables[varName] : 0;
        result.replace(match.position(), match.length(), to_string(value));
    }
    return result;
}

bool ConditionEvaluator::evaluate(GameEngine& engine, const string& condition) {
    vector<string> parts = tokenize(condition);

    // 物品持有检查
    if (!parts.empty() && parts[0] == "have" && parts.size() >= 2) {
        auto& inventory = engine.getInventoryManager().getItems();
        return any_of(inventory.begin(), inventory.end(), [&](const GameObject& item) {
            return item.name == parts[1] && item.getProperty("count", 1) > 0; 
        });
    }

    // 地点标记检查
    if (!parts.empty() && parts[0] == "去过" && parts.size() >= 2) {
        auto& markers = engine.getVisitedMarkers();
        return markers.find(parts[1]) != markers.end();
    }

    // 变量比较
    if (parts.size() == 3 && parts[1] == "is") {
        auto& variables = engine.getVariables();
        int lhs = variables.count(parts[0]) ? variables[parts[0]] : 0;
        int rhs = evaluateExpression(engine, parts[2]);
        return lhs == rhs;
    }

    // 通用比较运算符
    string condNoSpace = condition;
    condNoSpace.erase(
        remove_if(condNoSpace.begin(), condNoSpace.end(), ::isspace),
        condNoSpace.end()
    );

    regex compRegex(R"(^(\w+)(==|!=|>=|<=|>|<)(.+)$)");
    smatch match;
    if (regex_match(condNoSpace, match, compRegex)) {
        string varName = match[1];
        string op = match[2];
        string rhsExpr = match[3];

        auto& variables = engine.getVariables();
        int lhs = variables.count(varName) ? variables[varName] : 0;
        int rhs = evaluateExpression(engine, rhsExpr);

        if (op == "==") return lhs == rhs;
        if (op == "!=") return lhs != rhs;
        if (op == ">=") return lhs >= rhs;
        if (op == "<=") return lhs <= rhs;
        if (op == ">")  return lhs > rhs;
        if (op == "<")  return lhs < rhs;
    }

    throw runtime_error("无效条件格式: " + condition);
}

int ConditionEvaluator::evaluateExpression(GameEngine& engine, const string& expr) {
    try {
        string processed = replaceVariables(engine, expr);
        processed = regex_replace(processed, regex("\\s*([+\\-*/])\\s*"), "$1");
        
        // 纯数字直接返回
        try {
            size_t pos;
            int result = stoi(processed, &pos);
            if(pos == processed.size()) return result;
        } catch(...) {}
        
        // 简单二元运算
        regex opRegex(R"((\d+)([+*/-])(\d+))");
        smatch match;
        if(regex_match(processed, match, opRegex)) {
            int a = stoi(match[1]);
            int b = stoi(match[3]);
            char op = match[2].str()[0];
            switch(op) {
                case '+': return a + b;
                case '-': return a - b;
                case '*': return a * b;
                case '/': return b != 0 ? a / b : 0;
            }
        }
        return 0;
    } catch (const exception& e) {
        Log log("error.log");
        log.error("表达式解析错误: ", string(e.what()));
        return 0;
    }
}