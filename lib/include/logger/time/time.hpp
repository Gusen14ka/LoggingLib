#pragma once

#include <chrono>
#include <optional>
#include <string>

// format_timestamp: форматирует произвольный time_point в строку вида
// "ГГГГ-ММ-ДД чч:мм:сс:мсмсмс"
std::string format_timestamp(std::chrono::system_clock::time_point const& tp);

std::string current_timestamp();

// Ожидаемый формат timestamp_str: "2026-07-19 16:23:54:135"
std::optional<std::chrono::system_clock::time_point> parse_timestamp(std::string_view timestamp_str);