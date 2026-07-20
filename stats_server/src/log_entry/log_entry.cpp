#include <iomanip>

#include "log_entry.hpp"
#include "logger/logger.hpp"
#include "logger/log_level/log_level.hpp"
#include "logger/time/time.hpp"


LogEntry::LogEntry(LogLevel level,
    std::chrono::time_point<std::chrono::system_clock> timestamp, std::string text)
        : level(level), timestamp(timestamp), text(std::move(text)) {}



std::unique_ptr<LogEntry> LogEntry::parse(std::string const & message) {
    constexpr std::string_view separator = "] [";
    // размер пустого шаблона "[] [] []" равен 8
    if (message.size() <= 8 || message.front() != '[' || message.back() != ']') {
        return nullptr;
    }

    auto first_sep = message.find(separator);
    if (first_sep == std::string::npos) {
        return nullptr;
    }

    auto second_sep = message.find(separator, first_sep + separator.size());
    if (second_sep == std::string::npos) {
        return nullptr;
    }

    // first_sep - 1 >= 0 так как уже пройдена проверка message.front() == '['
    // => first_sep = message.find(separator) >= 1
    auto level_token = std::string_view(message.data() + 1, first_sep - 1);
    auto timestamp_token = std::string_view(
        message.data() + first_sep + separator.size(),
        second_sep - (first_sep + separator.size()));

    auto level = log_level_from_string(level_token);
    if (!level) {
        return nullptr;
    }

    auto timestamp = parse_timestamp(timestamp_token);
    if (!timestamp) {
        return nullptr;
    }

    auto text_start = second_sep + separator.size();
    // Делаем проверку чтобы точно не выйти за границы
    if (text_start >= message.size()) {
        return nullptr;
    }
    std::string text = message.substr(text_start, message.size() - text_start - 1);

    return std::unique_ptr<LogEntry>(new LogEntry(*level, *timestamp, std::move(text)));
}