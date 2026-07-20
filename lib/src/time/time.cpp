#include <iomanip>
#include <sstream>

#include "logger/time/time.hpp"


std::string format_timestamp(std::chrono::system_clock::time_point const& tp) {
    std::time_t time = std::chrono::system_clock::to_time_t(tp);

    std::tm tm_buf{};
    localtime_r(&time, &tm_buf);

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << ":"
        << std::setfill('0')
        << std::setw(3)
        << millis.count();

    return oss.str();
}

std::string current_timestamp(){
    return format_timestamp(std::chrono::system_clock::now());
}

std::optional<std::chrono::system_clock::time_point> parse_timestamp(std::string_view timestamp_str) {
    std::tm tm{};
    std::istringstream ss{std::string(timestamp_str)};  // istringstream не умеет напрямую в string_view

    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return std::nullopt;
    }

    char separator = 0;
    int millis = 0;
    ss >> separator;
    if (ss.fail() || separator != ':') {
        return std::nullopt;
    }
    ss >> millis;
    if (ss.fail() || millis < 0 || millis > 999) {
        return std::nullopt;
    }

    std::time_t time = std::mktime(&tm);
    if (time == static_cast<std::time_t>(-1)) {
        return std::nullopt;  // tm содержал невозможную дату (например, 32 февраля)
    }

    auto tp = std::chrono::system_clock::from_time_t(time);
    tp += std::chrono::milliseconds(millis);
    return tp;
}