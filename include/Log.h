#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <stdexcept>
#include <utility>

class Log {
public:
    explicit Log(const std::string& filename);
    ~Log() = default;

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    template<typename... Args>
    void write(Args&&... args);

private:
    std::string filename_;
    mutable std::mutex mutex_;
    std::ofstream out_stream_;

    void open_stream();
    void try_write();
    
    template<typename T, typename... Args>
    void write_impl(T&& first, Args&&... rest);
};

// 模板实现放在头文件中
template<typename... Args>
void Log::write(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    try_write();
    if (out_stream_.is_open()) {
        (out_stream_ << ... << std::forward<Args>(args)) << '\n';
        out_stream_.flush();
    }
}