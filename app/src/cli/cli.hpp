#pragma once
#include <optional>
#include <string>

enum class MessageLevel;

struct AppConfig {
    std::string const log_file_name;
    MessageLevel const default_level;
};

std::optional<AppConfig> parse_args(int argc, char* argv[]);

std::pair<std::string, std::optional<MessageLevel>> parse_log_level(std::string const& message);
