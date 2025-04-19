#pragma once
#include "../../common/Types.h"
#include "../../common/Exceptions.h"
#include "Condition.h"
#include "Expression.h"
#include <memory>
#include <regex>

namespace terrpg::core::scripting {

class Parser {
public:
    // 主解析方法
    static Condition parseCondition(const String& conditionStr);
    static std::unique_ptr<Expression> parseExpression(const String& exprStr);

    // 辅助方法
    static bool isCompoundCondition(const String& str);
    static bool isComparisonExpr(const String& str);
    
private:
    // 正则表达式模式
    static const std::regex VARIABLE_EXISTS_REGEX;
    static const std::regex INVENTORY_CHECK_REGEX;
    static const std::regex MARKER_CHECK_REGEX;
    static const std::regex COMPARISON_REGEX;
    static const std::regex COMPOUND_REGEX;

    // 子解析方法
    static Condition parseAtomicCondition(const String& str);
    static Condition parseCompoundCondition(const String& str);
};

} // namespace terrpg::core::scripting