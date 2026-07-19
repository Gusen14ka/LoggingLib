add_executable(log_app
        src/cli/cli.cpp
        src/log_worker/log_worker.cpp
        src/main.cpp
)

target_link_libraries(log_app PRIVATE logger)