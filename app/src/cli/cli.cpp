#include <memory>
#include <iostream>

#include "cli.hpp"
#include "logger/logger.hpp"

namespace {
    static std::optional<MessageLevel> log_level_from_string(std::string_view log_level) {
        std::string str(log_level);
        // std::transform(str.begin(), str.end(), str.begin(),
        //     [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (str == "debug")   return MessageLevel::DEBUG;
        if (str == "info")    return MessageLevel::INFO;
        if (str == "warning") return MessageLevel::WARNING;
        if (str == "error")   return MessageLevel::ERROR;
        return std::nullopt;
    }
}

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


std::pair<std::string, std::optional<MessageLevel>> parse_log_level(std::string const& message) {
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


