#include <memory>
#include <iostream>

#include "cli.hpp"
#include "logger/logger.hpp"
#include "logger/log_level/log_level.hpp"


std::optional<AppConfig> parse_args(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <log_file_name> <default_level>" << std::endl;
        std::cerr << "  default_level: DEBUG | INFO | WARNING | ERROR" << std::endl;
        return std::nullopt;
    }

    auto log_level = log_level_from_string(argv[2]);
    if (!log_level) {
        std::cerr << "Invalid log level: " << argv[2] << std::endl;
        std::cerr << "Expected one of: DEBUG, INFO, WARNING, ERROR" << std::endl;
        return std::nullopt;
    }

    return AppConfig{.log_file_name = argv[1], .default_level = log_level.value()};
}


std::pair<std::string, std::optional<LogLevel>> parse_log_level(std::string const& message) {
    auto space_pos = message.find(' ');
    if (space_pos != std::string::npos) {
        std::string_view first_token(message.data(), space_pos);
        if (auto level = log_level_from_string(first_token)) {
            std::string rest = message.substr(space_pos + 1);
            return {std::move(rest), level};
        }
    }
    return {message, std::nullopt};
}


