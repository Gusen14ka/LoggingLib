add_executable(stats_server_tests
        test_statistics.cpp
        test_log_entry.cpp
        test_cli.cpp
        test_print_stats.cpp
)

target_link_libraries(stats_server_tests PRIVATE stats_server_core gtest_main)

gtest_discover_tests(stats_server_tests)