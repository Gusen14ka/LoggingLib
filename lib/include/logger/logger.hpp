#pragma once

#include <fstream>
#include <functional>
#include <memory>

#include <logger/strategies/log_strategy.hpp>
#include <logger/log_level/log_level.hpp>

// Логирует текстовые сообщения с фильтрацией по уровню важности.
// Не бросает исключений: ошибки записи выводятся в stderr, программа продолжает работать.
// Создаётся через статические Logger::create(...) — конструктор приватный,
// это гарантирует, что успешно созданный объект всегда полностью работоспособен.
class Logger {
    LogLevel default_level_;
    std::unique_ptr<ILogStrategy> strategy_;
public:
    static std::unique_ptr<Logger> create(std::string const & log_file_name, LogLevel default_level);
    static std::unique_ptr<Logger> create(std::string const & host, int port, LogLevel default_level);
    static std::unique_ptr<Logger> create(std::unique_ptr<ILogStrategy> strategy, LogLevel default_level);

    ~Logger() = default;
    void log(std::string const & message, LogLevel const level);
    void log(std::string const & message);
    void change_default_level(LogLevel level);

    // TODO: Дописать все 6 к-тор и опер-ов
    Logger(Logger const & o) = delete;
    Logger& operator=(Logger const & o) = delete;

private:
    Logger(std::unique_ptr<ILogStrategy> strategy, LogLevel default_level);
    static std::unique_ptr<ILogStrategy> build(
        std::function<std::unique_ptr<ILogStrategy>(std::string&)> factory);
};

