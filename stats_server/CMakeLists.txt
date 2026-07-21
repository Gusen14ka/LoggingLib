add_executable(stats_server
        src/main.cpp
)

add_library(stats_server_core STATIC
        src/log_entry/log_entry.cpp
        src/cli/cli.cpp
        src/client_handler/client_handler.cpp
        src/client_registry/client_registry.cpp
        src/print_stats/print_stats.cpp
        src/statistics/statistics.cpp
        src/timer/timer.cpp
)

target_link_libraries(stats_server_core PUBLIC logger)
target_include_directories(stats_server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(stats_server PRIVATE stats_server_core)