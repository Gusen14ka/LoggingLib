add_library(logger
        src/logger.cpp
        src/strategies/file_log_strategy.cpp
        src/strategies/socket_log_strategy.cpp
        src/utils/error_utils.cpp
        src/time/time.cpp
        src/log_level/log_level.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(logger PUBLIC Threads::Threads)

target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)