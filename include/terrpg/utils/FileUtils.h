// include/terrpg/utils/FileUtils.h
#pragma once
#include "../common/Types.h"
#include "../common/Exceptions.h"

namespace terrpg::utils {

class FileUtils {
public:
    // 读取整个文件到字符串
    static String readFileToString(const common::Path& path);
    
    // 按行读取文件
    static Vector<String> readFileLines(const common::Path& path);
    
    // 检查文件是否存在
    static bool fileExists(const common::Path& path);
    
    // 获取文件扩展名（小写）
    static String getFileExtension(const common::Path& path);
};

} // namespace terrpg::utils