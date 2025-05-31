// include/Log.h

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
    void write(Args&&... args) const;

    template<typename... Args>
    void debug(Args&&... args) const {
        write("[debug] ", std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(Args&&... args) const {
        write("[info] ", std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(Args&&... args) const {
        write("[warn] ", std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(Args&&... args) const {
        write("[error] ", std::forward<Args>(args)...);
    }

private:
    std::string filename_;
    mutable std::mutex mutex_;
    mutable std::ofstream out_stream_;

    void open_stream() const;
    void try_write() const;
    
    template<typename T, typename... Args>
    void write_impl(T&& first, Args&&... rest);
};

// 模板实现放在头文件中
template<typename... Args>
void Log::write(Args&&... args) const {
    std::lock_guard<std::mutex> lock(mutex_);
    try_write();
    if (out_stream_.is_open()) {
        (out_stream_ << ... << std::forward<Args>(args)) << '\n';
        out_stream_.flush();
    }
}