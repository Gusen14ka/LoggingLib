#pragma once

#include <chrono>
#include <memory>
#include <string>


enum class LogLevel;

class LogEntry {
    public:
    // Поля неизменяемые, поэтому можно сделать их публичными, чтобы не писать лишних геттеров
    LogLevel const level;
    std::chrono::time_point<std::chrono::system_clock> const timestamp;
    std::string const text;
    
    // Ожидаемый формат message: "[<level>] [<timestamp>] [<text>]"
    static std::unique_ptr<LogEntry> parse(std::string const & message);

private:
    LogEntry(LogLevel level,
        std::chrono::time_point<std::chrono::system_clock> timestamp, std::string message);
};
