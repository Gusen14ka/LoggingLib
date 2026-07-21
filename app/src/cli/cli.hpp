#pragma once

#include <optional>
#include <string>

enum class LogLevel;

struct AppConfig {
    std::string const log_file_name;
    LogLevel const default_level;
};

std::optional<AppConfig> parse_args(int argc, char* argv[]);

std::pair<std::string, std::optional<LogLevel>> parse_log_level(std::string const& message);
