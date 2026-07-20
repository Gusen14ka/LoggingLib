#pragma once
#include <array>
#include <chrono>
#include <mutex>
#include <set>

#include "log_entry/log_entry.hpp"

struct Snapshot {
    size_t total_count = 0; // за всё время
    std::array<size_t, 4> per_level_count {}; // за всё время
    size_t last_hour_count = 0; // за последний час
    size_t min_length = SIZE_MAX; // только за последний час
    size_t max_length = 0; // за всё время
    size_t total_length = 0; // за всё время; для среднего = total_length / total_count

    bool operator==(const Snapshot &other) const;
    bool operator!=(const Snapshot &other) const;
};

class Statistics {
    Snapshot snapshot_;
    mutable std::mutex mtx_;
    std::multiset<std::chrono::system_clock::time_point> recent_timestamps_;

public:
    Statistics() = default;
    // Возвращает число зафиксированных записей
    size_t update(LogEntry const &entry);
    Snapshot snapshot() const;
    void print() const;

private:
    // Вызывается под локом в update(LogEntry const &entry)
    size_t pop_old_timestamps(std::chrono::system_clock::time_point now);
};


