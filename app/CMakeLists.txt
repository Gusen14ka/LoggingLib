add_executable(app
        src/main.cpp
)

# Создаём статическую таргет-библиотеку для линковки с тестами
add_library(app_core STATIC
        src/cli/cli.cpp
        src/log_worker/log_worker.cpp
)

# Добавляем публично, чтобы тесты видели
# Но из-за того что сам таргет статик, его не будет видно из других проектов
target_include_directories(app_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(app_core PUBLIC logger)

target_link_libraries(app PRIVATE app_core)