#include <gtest/gtest.h>

#include "logger/log_level/log_level.hpp"
#include "cli/cli.hpp"

namespace {
    char* to_cstr(std::string& s) { return s.data(); }
}

TEST(ParseLogLevelTest, ExtractsExplicitLevel) {
    auto result = parse_log_level("ERROR Connection refused");
    EXPECT_EQ(result.second.value(), LogLevel::ERROR);
    EXPECT_EQ(result.first, "Connection refused");
}

TEST(ParseLogLevelTest, NoLevelUsesWholeLineAsMessage) {
    auto result = parse_log_level("Server started normally");
    EXPECT_EQ(result.second, std::nullopt);
    EXPECT_EQ(result.first, "Server started normally");
}

TEST(ParseLogLevelTest, LevelCaseSensitivity) {
    // нижний регистр не распознаётся как уровень — вся строка становится сообщением
    auto result = parse_log_level("error lowercase should not match");
    EXPECT_EQ(result.second, std::nullopt);
    EXPECT_EQ(result.first, "error lowercase should not match");
}

TEST(ParseLogLevelTest, SingleWordMatchingLevelWithNoFollowupText) {
    // "ERROR" без пробела и текста после — трактуется как обычное сообщение,
    // не как "уровень ERROR с пустым сообщением" (осознанное решение, см. README)
    auto result = parse_log_level("ERROR");
    EXPECT_EQ(result.second, std::nullopt);
    EXPECT_EQ(result.first, "ERROR");
}

TEST(ParseLogLevelTest, EmptyLine) {
    auto result = parse_log_level("");
    EXPECT_EQ(result.second, std::nullopt);
    EXPECT_EQ(result.first, "");
}

TEST(ParseLogLevelTest, MultipleSpacesAfterLevel) {
    // задокументированное поведение: лишние пробелы после уровня не обрезаются
    auto result = parse_log_level("INFO   extra spaces");
    EXPECT_EQ(result.second.value(), LogLevel::INFO);
    EXPECT_EQ(result.first, "  extra spaces");
}

TEST(LogAppArgsTest, ParsesValidArguments)
{
    std::string argv0 = "logger";
    std::string argv1 = "app.log";
    std::string argv2 = "INFO";

    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2)};

    auto config = parse_args(3, argv);

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->log_file_name, "app.log");
    EXPECT_EQ(config->default_level, LogLevel::INFO);
}

TEST(LogAppArgsTest, RejectsWrongArgumentCount)
{
    std::string argv0 = "logger";
    std::string argv1 = "app.log";

    char* argv[] = {to_cstr(argv0), to_cstr(argv1)};

    EXPECT_EQ(parse_args(2, argv), std::nullopt);
}

TEST(LogAppArgsTest, RejectsInvalidLogLevel)
{
    std::string argv0 = "logger";
    std::string argv1 = "app.log";
    std::string argv2 = "TRACE";

    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2)};

    EXPECT_EQ(parse_args(3, argv), std::nullopt);
}
