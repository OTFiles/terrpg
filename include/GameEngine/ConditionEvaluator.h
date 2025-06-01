// include/ConditionEvaluator.h
#pragma once
#include <vector>
#include <string>

class GameEngine;

/**
 * @class ConditionEvaluator
 * @brief 游戏条件评估器，负责解析和评估游戏中的各种条件表达式
 * 
 * 该类提供静态方法用于：
 * 1. 检查玩家物品持有状态
 * 2. 验证地点访问标记
 * 3. 比较游戏变量值
 * 4. 执行算术表达式计算
 * 5. 处理游戏变量替换
 */
class ConditionEvaluator {
public:
    /**
     * @brief 评估游戏条件表达式
     * @param engine 游戏引擎引用，用于访问游戏状态
     * @param condition 要评估的条件字符串
     * @return 条件是否满足
     * 
     * 支持的条件类型：
     * - 物品持有检查: "have itemName"
     * - 地点访问检查: "去过 locationMarker"
     * - 变量相等检查: "varName is value"
     * - 通用比较表达式: "varName>=10" (支持 ==, !=, >, <, >=, <=)
     */
    static bool evaluate(GameEngine& engine, const std::string& condition);
    
    /**
     * @brief 计算算术表达式
     * @param engine 游戏引擎引用
     * @param expr 要计算的表达式字符串
     * @return 表达式计算结果
     * 
     * 功能特点：
     * - 支持变量替换（{varName}格式）
     * - 支持基本四则运算（+, -, *, /）
     * - 自动处理空格和变量替换
     */
    static int evaluateExpression(GameEngine& engine, const std::string& expr);

private:
    /**
     * @brief 分词函数
     * @param line 输入字符串
     * @return 分词后的字符串向量
     */
    static std::vector<std::string> tokenize(const std::string& line);
    
    /**
     * @brief 替换表达式中的变量
     * @param engine 游戏引擎引用
     * @param expr 包含变量的原始表达式
     * @return 变量替换后的表达式
     * 
     * 替换规则：
     * - 识别 {variableName} 格式的变量
     * - 将变量替换为其当前整数值
     * - 未定义变量默认为0
     */
    static std::string replaceVariables(GameEngine& engine, const std::string& expr);
};