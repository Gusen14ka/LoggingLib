#include <gtest/gtest.h>
#include "cli/cli.hpp"

namespace {
    char* to_cstr(std::string& s) { return s.data(); }
}

TEST(StatsServerArgsTest, ParsesValidArguments) {
    std::string argv0 = "stats_server", argv1 = "127.0.0.1", argv2 = "9000", argv3 = "30", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};

    auto config = parse_args(5, argv);
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->host, "127.0.0.1");
    EXPECT_EQ(config->port, 9000);
    EXPECT_EQ(config->report_interval, 30);
    EXPECT_EQ(config->report_messages_interval, 5);
}

TEST(StatsServerArgsTest, RejectsWrongArgumentCount) {
    std::string argv0 = "stats_server", argv1 = "127.0.0.1";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1)};
    EXPECT_EQ(parse_args(2, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsNonNumericPort) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "not_a_port", argv3 = "30", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsNegativePort) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "-1", argv3 = "30", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsPortAboveValidRange) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "70000", argv3 = "30", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsZeroReportInterval) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "9000", argv3 = "0", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsZeroMessagesInterval) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "9000", argv3 = "30", argv4 = "0";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsTrailingGarbageAfterPort) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "9000abc", argv3 = "30", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsTrailingGarbageAfterTimeInterval) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "9000", argv3 = "30abc", argv4 = "5";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}

TEST(StatsServerArgsTest, RejectsTrailingGarbageAfterMessageInterval) {
    std::string argv0 = "s", argv1 = "127.0.0.1", argv2 = "9000", argv3 = "30", argv4 = "5abc";
    char* argv[] = {to_cstr(argv0), to_cstr(argv1), to_cstr(argv2), to_cstr(argv3), to_cstr(argv4)};
    EXPECT_EQ(parse_args(5, argv), std::nullopt);
}