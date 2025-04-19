// src/utils/FileUtils.cpp
#include "utils/FileUtils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace terrpg;
using namespace terrpg::utils;

String FileUtils::readFileToString(const common::Path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw common::FileIOException(path.string(), "open");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Vector<String> FileUtils::readFileLines(const common::Path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw common::FileIOException(path.string(), "open");
    }
    
    Vector<String> lines;
    String line;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

bool FileUtils::fileExists(const common::Path& path) {
#if __has_include(<filesystem>)
    return fs::exists(path);
#else
    std::ifstream f(path.c_str());
    return f.good();
#endif
}

String FileUtils::getFileExtension(const common::Path& path) {
    String filename = path.filename().string();
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == String::npos) return "";
    
    String ext = filename.substr(dotPos + 1);
    return StringUtils::toLower(ext);
}