#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "logger/logger.hpp"
#include "logger/strategies/file_log_strategy.hpp"
#include "logger/strategies/socket_log_strategy.hpp"
#include "logger/time/time.hpp"


std::unique_ptr<ILogStrategy> Logger::build(
        std::function<std::unique_ptr<ILogStrategy>(std::string&)> factory) {
    std::string err;
    auto strategy = factory(err);
    if (!strategy) {
        std::cerr << "Error in Logger constructing: " << err << std::endl;
        std::cerr << "Following usage of this Logger will be affected" << std::endl;
    }
    return strategy;
}

std::unique_ptr<Logger> Logger::create(std::string const & log_file_name, LogLevel default_level) {
    auto strategy = build([&](std::string& err) {
        return FileLogStrategy::create(log_file_name, err);
    });
    if (strategy) {
        return std::unique_ptr<Logger>(new Logger(std::move(strategy), default_level));
    }
    return nullptr;
}

std::unique_ptr<Logger> Logger::create(std::string const & host, int port,
    LogLevel default_level) {
    auto strategy = build([&](std::string& err) {
        return SocketLogStrategy::create(host, port, err);
    });
    if (strategy) {
        return std::unique_ptr<Logger>(new Logger(std::move(strategy), default_level));
    }
    return nullptr;
}

std::unique_ptr<Logger> Logger::create(std::unique_ptr<ILogStrategy> strategy,
    LogLevel default_level) {
    if (strategy) {
        return std::unique_ptr<Logger>(new Logger(std::move(strategy), default_level));
    }
    return nullptr;
}

Logger::Logger(std::unique_ptr<ILogStrategy> strategy,
    const LogLevel default_level) :
        default_level_(default_level),
        strategy_(std::move(strategy)) {}

void Logger::log(std::string const & message, const LogLevel level){
    if (level < default_level_) return;

    std::ostringstream oss;
    oss << "["
        << to_string(level)
        << "] ["
        << current_timestamp()
        << "] ["
        << message
        << "]\n";

    if (std::string err; !strategy_->write(oss.str(), err)) {
        std::cerr << "Error in Logger: " << err << std::endl;
    }
}

void Logger::log(std::string const & message){
    log(message, default_level_);
}

void Logger::change_default_level(LogLevel level) {
    default_level_ = level;
}
