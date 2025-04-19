// include/terrpg/utils/StringUtils.h
#pragma once
#include "../common/Types.h"
#include <vector>

namespace terrpg::utils {

class StringUtils {
public:
    // 分割字符串
    static Vector<String> split(StringView str, char delimiter);
    
    // 去除两端空白
    static String trim(StringView str);
    
    // 转换为小写
    static String toLower(StringView str);
    
    // 替换变量占位符（如{name}）
    static String replaceVariables(StringView str, const Map<String, int>& variables);
    
    // 检查字符串是否以指定前缀开头
    static bool startsWith(StringView str, StringView prefix);
    
    // 检查字符串是否以指定后缀结尾
    static bool endsWith(StringView str, StringView suffix);
    
    // 安全转换为整数
    static int toInt(StringView str, bool& success);
};

} // namespace terrpg::utils