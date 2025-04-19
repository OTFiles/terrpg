// include/terrpg/common/Types.h
#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>

namespace terrpg::common {

// 字符串类型别名
using String = std::string;
using StringView = std::string_view;

// 容器类型模板
template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V>
using Map = std::map<K, V>;

template<typename T>
using Set = std::set<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

// 文件系统路径类型（兼容C++17）
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
using Path = fs::path;
#else
using Path = std::string; // 回退方案
#endif

} // namespace terrpg::common