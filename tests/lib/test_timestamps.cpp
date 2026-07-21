#include <gtest/gtest.h>
#include "logger/time/time.hpp"

TEST(TimestampUtils, ParseValidTimestamp) {
    auto tp = parse_timestamp("2026-07-19 16:23:54:135");
    ASSERT_TRUE(tp.has_value());
    // сверяем через format_timestamp — round-trip проще, чем вручную разбирать time_point
    EXPECT_EQ(format_timestamp(*tp), "2026-07-19 16:23:54:135");
}

TEST(TimestampUtils, ParseInvalidFormats) {
    EXPECT_EQ(parse_timestamp(""), std::nullopt);
    EXPECT_EQ(parse_timestamp("not a timestamp"), std::nullopt);
    EXPECT_EQ(parse_timestamp("2026-07-19 16:23:54"), std::nullopt);      // нет миллисекунд
    EXPECT_EQ(parse_timestamp("2026-13-19 16:23:54:135"), std::nullopt);  // невозможная дата (месяц)
    EXPECT_EQ(parse_timestamp("2026-07-32 16:23:54:135"), std::nullopt);  // невозможная дата (день)
    EXPECT_EQ(parse_timestamp("2026-07-19 24:23:54:135"), std::nullopt);  // невозможная дата (час)
    EXPECT_EQ(parse_timestamp("2026-07-19 16:60:54:135"), std::nullopt);  // невозможная дата (минута)
    EXPECT_EQ(parse_timestamp("2026-07-19 16:23:60:135"), std::nullopt);  // невозможная дата (секунда)
    EXPECT_EQ(parse_timestamp("2026-07-19 16:23:54:1000"), std::nullopt); // невозможная дата (миллисекунда)
    EXPECT_EQ(parse_timestamp("2026-02-29 16:23:54:1000"), std::nullopt); // невозможная дата (невисокосный год)
}

TEST(TimestampUtils, FormatCurrentTimeProducesParsableString) {
    auto now = std::chrono::system_clock::now();
    // отбрасываем под-миллисекундную точность, так как формат хранит только до мс
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

    auto str = format_timestamp(now_ms);
    auto parsed = parse_timestamp(str);

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(*parsed, now_ms);
}

TEST(TimestampUtils, CurentTimeProducesCorrectString) {
    auto cur = current_timestamp();
    // Отбрасываем миллисекунды (в них будет различие)
    auto sub_cur = cur.substr(0, cur.length() - 3);

    auto now = std::chrono::system_clock::now();
    // отбрасываем под-миллисекундную точность, так как формат хранит только до мс
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

    auto str_now = format_timestamp(now_ms);
    auto sub_str_now = sub_cur.substr(0, str_now.length() - 3);

    EXPECT_EQ(sub_cur, sub_str_now);
}