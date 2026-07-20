#include "print_stats.hpp"

void print_stats(Snapshot const& snap, std::ostream& out, std::string_view cause) {
    out << "--- Statistics " << cause << " ---\n"
        << "Total messages: " << snap.total_count << "\n"
        << "  DEBUG:   " << snap.per_level_count[0] << "\n"
        << "  INFO:    " << snap.per_level_count[1] << "\n"
        << "  WARNING: " << snap.per_level_count[2] << "\n"
        << "  ERROR:   " << snap.per_level_count[3] << "\n"
        << "Messages in last hour: " << snap.last_hour_count << "\n"
        << "Length — min: " << snap.min_length
        << ", max: " << snap.max_length
        << ", avg: " << (snap.total_count ? 1.0 * snap.total_length / snap.total_count : 0.0)
        << "\n" << std::endl;
}