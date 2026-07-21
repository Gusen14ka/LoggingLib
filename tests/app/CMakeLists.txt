add_executable(app_tests
        test_cli.cpp
)

target_link_libraries(app_tests PRIVATE app_core gtest_main)

gtest_discover_tests(app_tests)