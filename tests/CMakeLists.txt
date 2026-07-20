include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(tests
        lib/test_log_level.cpp
        lib/test_timestamps.cpp
        lib/test_error_utils.cpp
#        test_log_entry.cpp
#        test_statistics.cpp
#        test_logger.cpp
)

target_link_libraries(tests PRIVATE logger gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)