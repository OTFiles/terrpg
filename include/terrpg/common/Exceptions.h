// include/terrpg/common/Exceptions.h
#pragma once
#include <stdexcept>
#include <string>

namespace terrpg::common {

class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& msg)
        : std::runtime_error("[Game] " + msg) {}
};

class FileIOException : public GameException {
public:
    explicit FileIOException(const std::string& path, const std::string& action)
        : GameException("File " + action + " failed: " + path) {}
};

class ParseException : public GameException {
public:
    ParseException(const std::string& msg, int line = -1)
        : GameException(msg + (line != -1 ? " at line " + std::to_string(line) : "")) {}
};

class CommandException : public GameException {
public:
    explicit CommandException(const std::string& cmd, const std::string& reason)
        : GameException("Command '" + cmd + "' error: " + reason) {}
};

class ScriptException : public GameException {
public:
    ScriptException(const std::string& msg, int line)
        : GameException("Script error at line " + std::to_string(line) + ": " + msg) {}
};

} // namespace terrpg::common