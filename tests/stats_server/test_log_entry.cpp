#include <gtest/gtest.h>
#include "log_entry/log_entry.hpp"
#include "logger/log_level/log_level.hpp"

TEST(LogEntryTest, ParsesValidLine) {
    auto entry = LogEntry::parse("[ERROR] [2026-07-19 16:23:54:135] [Connection refused]");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->level, LogLevel::ERROR);
    EXPECT_EQ(entry->text, "Connection refused");
}

TEST(LogEntryTest, ParsesEmptyMessageText) {
    auto entry = LogEntry::parse("[INFO] [2026-07-19 16:23:54:135] []");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->text, "");
}

TEST(LogEntryTest, RejectsMissingBrackets) {
    EXPECT_EQ(LogEntry::parse("no brackets at all"), nullptr);
    EXPECT_EQ(LogEntry::parse(""), nullptr);
}

TEST(LogEntryTest, RejectsInvalidLevel) {
    EXPECT_EQ(LogEntry::parse("[NOTLEVEL] [2026-07-19 16:23:54:135] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsInvalidTimestamp) {
    EXPECT_EQ(LogEntry::parse("[INFO] [not-timestamp] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsMissingClosingBracketForLevel) {
    EXPECT_EQ(LogEntry::parse("[INFO [2026-07-19 16:23:54:135] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsMissingOpeningBracketForLevel) {
    EXPECT_EQ(LogEntry::parse("INFO] [2026-07-19 16:23:54:135] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsMissingClosingBracketForTimestamp) {
    EXPECT_EQ(LogEntry::parse("[INFO] 2026-07-19 16:23:54:135] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsMissingOpeningBracketForTimestamp) {
    EXPECT_EQ(LogEntry::parse("[INFO] [2026-07-19 16:23:54:135 [text]"), nullptr);
}

TEST(LogEntryTest, RejectsMissingClosingBracketForText) {
    EXPECT_EQ(LogEntry::parse("[INFO] [2026-07-19 16:23:54:135] text]"), nullptr);
}

TEST(LogEntryTest, RejectsMissingOpeningBracketForText) {
    EXPECT_EQ(LogEntry::parse("[INFO] [2026-07-19 16:23:54:135] [text"), nullptr);
}

TEST(LogEntryTest, RejectsLineWithoutLogLevel) {
    EXPECT_EQ(LogEntry::parse("[2026-07-19 16:23:54:135] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsLineWithoutTimestamp) {
    EXPECT_EQ(LogEntry::parse("[INFO] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsLineWithoutText) {
    EXPECT_EQ(LogEntry::parse("[INFO] [2026-07-19 16:23:54:135]"), nullptr);
}

TEST(LogEntryTest, RejectsLineNotStartingWithBracket) {
    EXPECT_EQ(LogEntry::parse("garbage[INFO] [2026-07-19 16:23:54:135] [text]"), nullptr);
}

TEST(LogEntryTest, RejectsLineNotEndingWithBracket) {
    EXPECT_EQ(LogEntry::parse("[INFO] [2026-07-19 16:23:54:135] [text]trailing"), nullptr);
}

TEST(LogEntryTest, MessageTextCanContainBracketsAndSpecialCharacters) {
    // текст сообщения сам может содержать [ ], это не должно ломать парсинг
    auto entry = LogEntry::parse("[WARNING] [2026-07-19 16:23:54:135] [array[0] = [1, 2, 3]]");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->text, "array[0] = [1, 2, 3]");
}

TEST(LogEntryTest, VeryShortValidLine) {
    // граничный случай: минимально возможная валидная строка
    auto entry = LogEntry::parse("[DEBUG] [2026-01-01 00:00:00:000] [x]");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->level, LogLevel::DEBUG);
    EXPECT_EQ(entry->text, "x");
}