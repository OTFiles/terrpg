#include "Log.h"
#include <chrono>
#include <iomanip>

Log::Log(const std::string& filename) 
    : filename_(filename) 
{
    open_stream();
    if (!out_stream_.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }
}

void Log::open_stream() {
    out_stream_.open(filename_, std::ios::app);
}

void Log::try_write() {
    if (!out_stream_.good()) {
        out_stream_.clear();
        open_stream();
    }
}