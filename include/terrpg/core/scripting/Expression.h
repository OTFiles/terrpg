#pragma once
#include "../../common/Types.h"
#include <memory>

namespace terrpg::core::scripting {

class Expression {
public:
    virtual ~Expression() = default;
    virtual int evaluate(const ScriptBlock::ExecutionContext& ctx) const = 0;
};

class VariableExpression : public Expression {
public:
    explicit VariableExpression(String varName);
    int evaluate(const ScriptBlock::ExecutionContext& ctx) const override;
private:
    String varName_;
};

class LiteralExpression : public Expression {
public:
    explicit LiteralExpression(int value);
    int evaluate(const ScriptBlock::ExecutionContext& ctx) const override;
private:
    int value_;
};

class BinaryExpression : public Expression {
public:
    enum class Op { ADD, SUB, MUL, DIV };
    
    BinaryExpression(std::unique_ptr<Expression> lhs,
                    Op op,
                    std::unique_ptr<Expression> rhs);
    int evaluate(const ScriptBlock::ExecutionContext& ctx) const override;

private:
    std::unique_ptr<Expression> lhs_;
    Op op_;
    std::unique_ptr<Expression> rhs_;
};

} // namespace terrpg::core::scripting