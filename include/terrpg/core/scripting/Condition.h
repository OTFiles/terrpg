#pragma once
#include "../../common/Types.h"
#include "../../common/Exceptions.h"
#include <variant>
#include <vector>

namespace terrpg::core::scripting {

class Expression;

class Condition {
public:
    enum class CompoundOp { AND, OR };
    enum class ComparisonOp { EQ, NE, GT, LT, GE, LE };
    
    // 条件类型
    struct VariableCheck {
        String varName;
        bool exists;
    };
    
    struct InventoryCheck {
        String itemId;
        bool hasItem;
    };
    
    struct MarkerCheck {
        String markerId;
        bool visited;
    };
    
    struct ValueComparison {
        std::unique_ptr<Expression> lhs;
        ComparisonOp op;
        std::unique_ptr<Expression> rhs;
    };
    
    struct CompoundCondition {
        CompoundOp op;
        std::vector<Condition> conditions;
    };

    using ConditionVariant = std::variant<
        VariableCheck,
        InventoryCheck,
        MarkerCheck,
        ValueComparison,
        CompoundCondition
    >;

    // 构建方法
    static Condition makeVariableExists(const String& varName);
    static Condition makeInventoryCheck(const String& itemId, bool shouldHave);
    static Condition makeMarkerCheck(const String& markerId, bool shouldVisited);
    static Condition makeComparison(std::unique_ptr<Expression> lhs, 
                                   ComparisonOp op,
                                   std::unique_ptr<Expression> rhs);
    static Condition makeCompound(CompoundOp op, std::vector<Condition> conditions);

    // 评估入口
    bool evaluate(const class ScriptBlock::ExecutionContext& context) const;

private:
    explicit Condition(ConditionVariant condition);
    ConditionVariant condition_;
};

} // namespace terrpg::core::scripting