#include <gtest/gtest.h>
#include "logger/log_level/log_level.hpp"

TEST(LogLevel, ParseValidLevels) {
    EXPECT_EQ(log_level_from_string("DEBUG"), LogLevel::DEBUG);
    EXPECT_EQ(log_level_from_string("INFO"), LogLevel::INFO);
    EXPECT_EQ(log_level_from_string("WARNING"), LogLevel::WARNING);
    EXPECT_EQ(log_level_from_string("ERROR"), LogLevel::ERROR);
}

TEST(LogLevel, ParseInvalidLevelReturnsNullopt) {
    EXPECT_EQ(log_level_from_string("info"), std::nullopt);       // регистр важен
    EXPECT_EQ(log_level_from_string("TRACE"), std::nullopt);       // несуществующий уровень
    EXPECT_EQ(log_level_from_string(""), std::nullopt);            // пустая строка
    EXPECT_EQ(log_level_from_string("INFO "), std::nullopt);       // лишний пробел — не точное совпадение
}

TEST(LogLevel, ToStringRoundTrip) {
    for (auto level : {LogLevel::DEBUG, LogLevel::INFO,
                        LogLevel::WARNING, LogLevel::ERROR}) {
        auto str = to_string(level);
        auto parsed = log_level_from_string(str);
        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(*parsed, level);
    }
}