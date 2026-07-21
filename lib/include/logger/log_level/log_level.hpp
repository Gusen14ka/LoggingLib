#pragma once

#include <optional>
#include <string_view>


enum class LogLevel {DEBUG, INFO, WARNING, ERROR};

std::string_view to_string(LogLevel const & level);
std::optional<LogLevel> log_level_from_string(std::string_view log_level);
