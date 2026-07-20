add_executable(stats_server
        src/main.cpp
        src/log_entry/log_entry.cpp
        src/cli/cli.cpp
        src/client_handler/client_handler.cpp
        src/client_registry/client_registry.cpp
        src/print_stats/print_stats.cpp
        src/statistics/statistics.cpp
        src/timer/timer.cpp
)

target_link_libraries(stats_server PRIVATE logger)
target_include_directories(stats_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)