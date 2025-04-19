#include "Parser.h"
#include "Expression.h"
#include <sstream>

namespace terrpg::core::scripting {

// 正则表达式定义
const std::regex Parser::VARIABLE_EXISTS_REGEX(R"(^exists\s+(\w+)$)");
const std::regex Parser::INVENTORY_CHECK_REGEX(R"(^have\s+(\w+)$)");
const std::regex Parser::MARKER_CHECK_REGEX(R"(^visited\s+(\w+)$)");
const std::regex Parser::COMPARISON_REGEX(
    R"(^(.+?)\s*(==|!=|>=|<=|>|<)\s*(.+)$)"
);
const std::regex Parser::COMPOUND_REGEX(
    R"(^\((.*)\)\s+(and|or)\s+\((.*)\)$)"
);

Condition Parser::parseCondition(const String& conditionStr) {
    const String trimmed = StringUtils::trim(conditionStr);
    
    if(isCompoundCondition(trimmed)) {
        return parseCompoundCondition(trimmed);
    }
    return parseAtomicCondition(trimmed);
}

bool Parser::isCompoundCondition(const String& str) {
    return std::regex_search(str, COMPOUND_REGEX);
}

Condition Parser::parseAtomicCondition(const String& str) {
    std::smatch match;
    
    // 检查变量存在性
    if(std::regex_match(str, match, VARIABLE_EXISTS_REGEX)) {
        return Condition::makeVariableExists(match[1].str());
    }
    
    // 检查物品
    if(std::regex_match(str, match, INVENTORY_CHECK_REGEX)) {
        return Condition::makeInventoryCheck(match[1].str(), true);
    }
    
    // 检查标记点
    if(std::regex_match(str, match, MARKER_CHECK_REGEX)) {
        return Condition::makeMarkerCheck(match[1].str(), true);
    }
    
    // 数值比较
    if(std::regex_match(str, match, COMPARISON_REGEX)) {
        auto lhs = parseExpression(match[1].str());
        auto rhs = parseExpression(match[3].str());
        const String opStr = match[2].str();
        
        Condition::ComparisonOp op;
        if(opStr == "==") op = Condition::ComparisonOp::EQ;
        else if(opStr == "!=") op = Condition::ComparisonOp::NE;
        else if(opStr == ">") op = Condition::ComparisonOp::GT;
        else if(opStr == "<") op = Condition::ComparisonOp::LT;
        else if(opStr == ">=") op = Condition::ComparisonOp::GE;
        else if(opStr == "<=") op = Condition::ComparisonOp::LE;
        else throw common::ParseException("Invalid comparison operator: " + opStr);
        
        return Condition::makeComparison(std::move(lhs), op, std::move(rhs));
    }
    
    throw common::ParseException("Unrecognized condition format: " + str);
}

Condition Parser::parseCompoundCondition(const String& str) {
    std::smatch match;
    if(!std::regex_match(str, match, COMPOUND_REGEX)) {
        throw common::ParseException("Malformed compound condition: " + str);
    }
    
    Condition::CompoundOp op = (match[2].str() == "and") ? 
        Condition::CompoundOp::AND : 
        Condition::CompoundOp::OR;
    
    std::vector<Condition> conditions;
    conditions.reserve(2);
    
    conditions.push_back(parseCondition(match[1].str()));
    conditions.push_back(parseCondition(match[3].str()));
    
    return Condition::makeCompound(op, std::move(conditions));
}

std::unique_ptr<Expression> Parser::parseExpression(const String& exprStr) {
    // 实现表达式解析器（示例实现）
    return std::make_unique<LiteralExpression>(0); // 实际需要完整表达式解析
}

} // namespace terrpg::core::scripting