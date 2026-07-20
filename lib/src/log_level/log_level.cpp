#include <string>

#include "logger/log_level/log_level.hpp"


std::string_view to_string(LogLevel const & level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
    }

    return "UNKNOWN";
}

std::optional<LogLevel> log_level_from_string(std::string_view log_level) {
    if (log_level == "DEBUG")   return LogLevel::DEBUG;
    if (log_level == "INFO")    return LogLevel::INFO;
    if (log_level == "WARNING") return LogLevel::WARNING;
    if (log_level == "ERROR")   return LogLevel::ERROR;
    return std::nullopt;
}
