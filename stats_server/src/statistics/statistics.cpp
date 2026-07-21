#include "statistics.hpp"


// Snapshot возвращается целиком, не отдельным счётчиком: если бы вызывающий
// код делал отдельный update() + snapshot(), между ними лок отпускается,
// и параллельный update() из другого потока может "увести" total_count —
// триггер на N-е сообщение промахивается. Возврат из-под одного лока это устраняет.
Snapshot Statistics::update(LogEntry const &entry) {
    std::scoped_lock<std::mutex> lock(mtx_);
    snapshot_.total_count++;
    snapshot_.per_level_count[static_cast<size_t>(entry.level)]++;
    size_t length = entry.text.size();
    snapshot_.min_length = std::min(snapshot_.min_length, length);
    snapshot_.max_length = std::max(snapshot_.max_length, length);
    snapshot_.total_length += length;

    recent_timestamps_.insert(entry.timestamp);
    snapshot_.last_hour_count = pop_old_timestamps(std::chrono::system_clock::now());

    return snapshot_;
}

size_t Statistics::pop_old_timestamps(std::chrono::system_clock::time_point now) {
    auto cut_off = now - std::chrono::hours(1);
    auto first_valid = recent_timestamps_.lower_bound(cut_off);
    // Удаляя "записи", которые случились раньше чем час назад,
    // находим число сообщений пришедших за последний час
    recent_timestamps_.erase(recent_timestamps_.begin(), first_valid);
    return recent_timestamps_.size();
}

Snapshot Statistics::snapshot() const {
    std::scoped_lock<std::mutex> lock(mtx_);
    return snapshot_;
}

bool Snapshot::operator==(const Snapshot &other) const {
    if (this == &other) return true;
    return total_count == other.total_count
        && total_length == other.total_length
        && per_level_count == other.per_level_count
        && last_hour_count == other.last_hour_count
        && min_length == other.min_length
        && max_length == other.max_length;
}

bool Snapshot::operator!=(const Snapshot &other) const {
    return !(*this == other);
}
