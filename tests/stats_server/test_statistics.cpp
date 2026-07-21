#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>
#include "statistics/statistics.hpp"
#include "log_entry/log_entry.hpp"
#include "logger/time/time.hpp"
#include "logger/log_level/log_level.hpp"

namespace {

// Строит строку "[LEVEL] [timestamp] [text]" для заданного смещения от текущего момента,
// затем парсит её через LogEntry::parse — так тест использует тот же путь разбора,
// что и реальный клиент по сокету, вместо обхода приватного конструктора LogEntry.
std::unique_ptr<LogEntry> make_entry(LogLevel level,
                                       std::chrono::system_clock::duration offset_from_now,
                                       std::string const& text) {
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() + offset_from_now);
    std::string line = "[" + std::string(to_string(level)) + "] ["
                        + format_timestamp(tp) + "] [" + text + "]";
    return LogEntry::parse(line);
}

}  // namespace

TEST(StatisticsTest, TracksTotalCountAndPerLevel) {
    Statistics stats;

    auto e1 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "a");
    auto e2 = make_entry(LogLevel::ERROR, std::chrono::seconds(0), "bb");
    auto e3 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "ccc");
    ASSERT_TRUE(e1 && e2 && e3);

    stats.update(*e1);
    stats.update(*e2);
    stats.update(*e3);

    auto snap = stats.snapshot();
    EXPECT_EQ(snap.total_count, 3);
    EXPECT_EQ(snap.per_level_count[static_cast<size_t>(LogLevel::INFO)], 2);
    EXPECT_EQ(snap.per_level_count[static_cast<size_t>(LogLevel::ERROR)], 1);
    EXPECT_EQ(snap.per_level_count[static_cast<size_t>(LogLevel::DEBUG)], 0);
}

TEST(StatisticsTest, TracksMinMaxAvgLength) {
    Statistics stats;

    auto e1 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "a");       // длина 1
    auto e2 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "abcde");   // длина 5
    auto e3 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "abc");     // длина 3
    ASSERT_TRUE(e1 && e2 && e3);

    stats.update(*e1);
    stats.update(*e2);
    stats.update(*e3);

    auto snap = stats.snapshot();
    EXPECT_EQ(snap.min_length, 1);
    EXPECT_EQ(snap.max_length, 5);
    EXPECT_DOUBLE_EQ(static_cast<double>(snap.total_length) / snap.total_count, 3.0);  // (1+5+3)/3
}

TEST(StatisticsTest, LastHourCountExcludesEntriesOlderThanAnHour) {
    Statistics stats;

    // одно сообщение "сейчас", одно — час и одна минута назад (за пределами окна),
    // одно — 30 минут назад (в пределах окна)
    auto recent = make_entry(LogLevel::INFO, std::chrono::seconds(0), "recent");
    auto old = make_entry(LogLevel::INFO, -std::chrono::hours(1) - std::chrono::minutes(1), "old");
    auto within_window = make_entry(LogLevel::INFO, -std::chrono::minutes(30), "within");
    ASSERT_TRUE(recent && old && within_window);

    stats.update(*old);
    stats.update(*within_window);
    stats.update(*recent);

    auto snap = stats.snapshot();
    EXPECT_EQ(snap.total_count, 3);          // все три учтены в all-time статистике
    EXPECT_EQ(snap.last_hour_count, 2);      // только within_window и recent — старое исключено
}

TEST(StatisticsTest, UpdateReturnsRunningTotalCount) {
    Statistics stats;
    auto e1 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "a");
    auto e2 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "b");
    ASSERT_TRUE(e1 && e2);

    EXPECT_EQ(stats.update(*e1).total_count, 1);
    EXPECT_EQ(stats.update(*e2).total_count, 2);
}

TEST(StatisticsTest, SnapshotEqualityReflectsState) {
    Statistics stats;
    auto empty_snapshot = stats.snapshot();

    auto e1 = make_entry(LogLevel::INFO, std::chrono::seconds(0), "x");
    ASSERT_TRUE(e1);
    stats.update(*e1);

    auto after_update = stats.snapshot();

    EXPECT_FALSE(empty_snapshot == after_update);  // состояние изменилось — снимки должны различаться

    auto second_read = stats.snapshot();
    EXPECT_TRUE(after_update == second_read);       // без изменений между вызовами — снимки идентичны
}

TEST(StatisticsTest, ConcurrentUpdatesReturnConsistentSnapshots) {
    Statistics stats;
    constexpr int kThreads = 8;
    constexpr int kMessagesPerThread = 50;

    std::vector<std::thread> threads;
    std::mutex seen_mtx;
    std::set<size_t> seen_totals;  // все total_count, реально возвращённые из update()

    for (int t = 0; t < kThreads; t++) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kMessagesPerThread; i++) {
                auto entry = make_entry(LogLevel::INFO, std::chrono::seconds(0),
                                         "msg " + std::to_string(t) + "-" + std::to_string(i));
                ASSERT_TRUE(entry);
                auto snap = stats.update(*entry);

                std::scoped_lock lock(seen_mtx);
                seen_totals.insert(snap.total_count);
            }
        });
    }
    for (auto& th : threads) th.join();

    // Если бы update() и snapshot() были раздельными вызовами,
    // некоторые значения total_count были бы пропущены (два потока увидели бы
    // одно и то же "уехавшее" число). При корректной реализации КАЖДОЕ число
    // от 1 до kThreads*kMessagesPerThread встречается ровно один раз.
    EXPECT_EQ(seen_totals.size(), static_cast<size_t>(kThreads * kMessagesPerThread));
    EXPECT_EQ(*seen_totals.begin(), 1);
    EXPECT_EQ(*seen_totals.rbegin(), static_cast<size_t>(kThreads * kMessagesPerThread));
}