#pragma once

#include <fstream>
#include <functional>
#include <memory>

#include <logger/strategies/log_strategy.hpp>

enum class MessageLevel {DEBUG, INFO, WARNING, ERROR};

class Logger {
    MessageLevel default_level_;
    std::unique_ptr<ILogStrategy> strategy_;
public:
    static std::unique_ptr<Logger> create(std::string const & log_file_name, MessageLevel default_level);
    static std::unique_ptr<Logger> create(std::string const & host, int port, MessageLevel default_level);
    static std::unique_ptr<Logger> create(std::unique_ptr<ILogStrategy> strategy, MessageLevel default_level);

    ~Logger() = default;
    void log(std::string const & message, const MessageLevel level);
    void log(std::string const & message);
    void change_default_level(MessageLevel level);

    // TODO: Дописать все 6 к-тор и опер-ов
    Logger(Logger const & o) = delete;
    Logger& operator=(Logger const & o) = delete;

private:
    std::string current_timestamp();
    Logger(std::unique_ptr<ILogStrategy> strategy, MessageLevel default_level);
    static std::unique_ptr<ILogStrategy> build(
        std::function<std::unique_ptr<ILogStrategy>(std::string&)> factory);
};