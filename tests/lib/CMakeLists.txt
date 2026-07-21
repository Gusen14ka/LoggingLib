add_executable(lib_tests
        test_log_level.cpp
        test_timestamps.cpp
        test_error_utils.cpp
        test_logger.cpp
)

target_link_libraries(lib_tests PRIVATE logger gtest_main)

gtest_discover_tests(lib_tests)