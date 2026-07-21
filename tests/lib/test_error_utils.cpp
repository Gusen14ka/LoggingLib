#include <gtest/gtest.h>
#include <logger/utils/error_utils.hpp>


TEST(StrerrorSafe, ReturnsMessageForKnownError)
{
    EXPECT_FALSE(strerror_safe(EINVAL).empty());
}