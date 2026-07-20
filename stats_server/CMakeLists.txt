add_executable(stats_server
        src/main.cpp
        src/log_entry/log_entry.cpp
)

target_link_libraries(stats_server PRIVATE logger)
target_include_directories(stats_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)