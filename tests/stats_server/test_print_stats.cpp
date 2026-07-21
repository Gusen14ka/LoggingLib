#include <gtest/gtest.h>
#include <sstream>
#include "print_stats/print_stats.hpp"
#include "statistics/statistics.hpp"

TEST(StatsPrinterTest, OutputContainsAllKeyNumbers) {
    Snapshot snap;
    snap.total_count = 42;
    snap.per_level_count = {5, 10, 15, 12};  // DEBUG, INFO, WARNING, ERROR
    snap.last_hour_count = 30;
    snap.min_length = 3;
    snap.max_length = 100;
    snap.total_length = 42 * 20;  // avg = 20.0

    std::ostringstream out;
    print_stats(snap, out, "test");
    std::string text = out.str();

    EXPECT_NE(text.find("42"), std::string::npos);   // total
    EXPECT_NE(text.find("30"), std::string::npos);   // last hour
    EXPECT_NE(text.find("3"), std::string::npos);    // min
    EXPECT_NE(text.find("100"), std::string::npos);  // max
    EXPECT_NE(text.find("20"), std::string::npos);   // avg
}

TEST(StatsPrinterTest, HandlesEmptyStatisticsWithoutCrashing) {
    Snapshot snap{};  // всё по нулям — total_count == 0
    std::ostringstream out;

    EXPECT_NO_THROW(print_stats(snap, out, "test"));
    // проверяем именно что не было деления на ноль/NaN в выводе среднего
    EXPECT_EQ(out.str().find("nan"), std::string::npos);
    EXPECT_EQ(out.str().find("-nan"), std::string::npos);
    EXPECT_EQ(out.str().find("inf"), std::string::npos);
}

TEST(StatsPrinterTest, DoesNotThrowForAnyInput) {
    Snapshot snap;
    snap.total_count = 1;
    snap.per_level_count = {0, 1, 0, 0};
    snap.last_hour_count = 1;
    snap.min_length = 5;
    snap.max_length = 5;
    snap.total_length = 5;

    std::ostringstream out;
    EXPECT_NO_THROW(print_stats(snap, out, "test"));
    EXPECT_FALSE(out.str().empty());
}