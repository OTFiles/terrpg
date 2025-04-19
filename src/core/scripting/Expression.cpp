#include "Expression.h"
#include "../common/Exceptions.h"
#include <stdexcept>

namespace terrpg::core::scripting {

//=== VariableExpression ===//
VariableExpression::VariableExpression(String varName)
    : varName_(std::move(varName)) {}

int VariableExpression::evaluate(const ScriptBlock::ExecutionContext& ctx) const {
    auto it = ctx.variables.find(varName_);
    if(it == ctx.variables.end()) {
        throw common::ScriptException("Undefined variable: " + varName_);
    }
    return it->second;
}

//=== LiteralExpression ===//
LiteralExpression::LiteralExpression(int value)
    : value_(value)) {}

int LiteralExpression::evaluate(const ScriptBlock::ExecutionContext&) const {
    return value_;
}

//=== BinaryExpression ===//
BinaryExpression::BinaryExpression(std::unique_ptr<Expression> lhs,
                                   Op op,
                                   std::unique_ptr<Expression> rhs)
    : lhs_(std::move(lhs)),
      op_(op),
      rhs_(std::move(rhs)) {}

int BinaryExpression::evaluate(const ScriptBlock::ExecutionContext& ctx) const {
    const int left = lhs_->evaluate(ctx);
    const int right = rhs_->evaluate(ctx);
    
    switch(op_) {
        case Op::ADD: return left + right;
        case Op::SUB: return left - right;
        case Op::MUL: return left * right;
        case Op::DIV: {
            if(right == 0) {
                throw common::ScriptException("Division by zero");
            }
            return left / right;
        }
        default:
            throw common::ScriptException("Invalid operator");
    }
}

} // namespace terrpg::core::scripting