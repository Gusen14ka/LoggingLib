#include <iostream>
#include <charconv>
#include <cstring>

#include "cli.hpp"

#include <cstdint>

namespace {
    template <typename T>
    std::optional<T> parse_number(std::string_view token, std::string_view field_name) {
        T value;
        auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);

        if (ec != std::errc{}) {
            std::cerr << "Invalid " << field_name << ": '" << token << "'" << std::endl;
            return std::nullopt;
        }
        if (ptr != token.data() + token.size()) {
            std::cerr << "Trailing characters after " << field_name << ": '" << token << "'" << std::endl;
            return std::nullopt;
        }
        return value;
    }
}


std::optional<AppConfig> parse_args(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
            << " <host> <port> <report_interval> <report_messages_interval>" << std::endl;
        std::cerr << "  default_level: DEBUG | INFO | WARNING | ERROR" << std::endl;
        return std::nullopt;
    }

    // Делая парсинг в конкретный тип автоматически удовлетворяем ограничениям (или их части)
    auto port =
        parse_number<uint16_t>(argv[2], "port"); // 0...65535
    auto report_interval =
        parse_number<size_t>(argv[3], "report interval (T)"); // >=0
    auto report_messages_interval =
        parse_number<size_t>(argv[4], "report messages interval (N)"); // >=0

    if (!port || !report_interval || !report_messages_interval) {
        return std::nullopt;
    }
    if (*report_interval <= 0) {
        std::cerr << "Report interval (T) must be positive: " << *report_interval << std::endl;
        return std::nullopt;
    }
    if (*report_messages_interval <= 0) {
        std::cerr << "Report messages interval (N) must be positive: " << *report_messages_interval << std::endl;
        return std::nullopt;
    }

    return AppConfig{argv[1], *port,
        static_cast<size_t>(*report_messages_interval),
        static_cast<size_t>(*report_interval)};

}
