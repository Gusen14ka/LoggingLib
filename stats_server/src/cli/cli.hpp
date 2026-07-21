#pragma once

#include <optional>
#include <string>


struct AppConfig {
    std::string host;
    int port;
    size_t report_messages_interval;
    size_t report_interval;

};

std::optional<AppConfig> parse_args(int argc, char* argv[]);