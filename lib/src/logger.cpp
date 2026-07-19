#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "logger/logger.hpp"
#include "logger/strategies/file_log_strategy.hpp"
#include "logger/strategies/socket_log_strategy.hpp"

namespace {
    std::string to_string(MessageLevel const level){
        switch (level) {
            case MessageLevel::DEBUG: return "DEBUG";
            case MessageLevel::INFO: return "INFO";
            case MessageLevel::WARNING: return "WARNING";
            case MessageLevel::ERROR: return "ERROR";
        }

        return "UNKNOWN";
    }
}

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

Logger::Logger(std::string const & log_file_name,
    const MessageLevel default_level) :
        Logger(build([&](std::string& err) {
            return FileLogStrategy::create(log_file_name, err);
        }), default_level) {}

Logger::Logger(std::string const & host, int port,
    const MessageLevel default_level) :
        Logger(build([&](std::string& err) {
            return SocketLogStrategy::create(host, port, err);
        }),default_level) {}

Logger::Logger(std::unique_ptr<ILogStrategy> strategy,
    const MessageLevel default_level) :
        default_level_(default_level),
        strategy_(std::move(strategy)) {}

void Logger::log(std::string const & message, const MessageLevel level){
    if (level < default_level_) return;
    if (!strategy_) {
        std::cerr << "Attempt to use Logger with null strategy" << std::endl;
        return;
    }

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

void Logger::change_default_level(MessageLevel level) {
    default_level_ = level;
}

std::string Logger::current_timestamp(){
    const auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_buf {};
    #ifdef _WIN32
        localtime_s(&tm_buf, &time);
    #else
        localtime_r(&time, &tm_buf);
    #endif

    const auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << ":"
        << std::setfill('0')
        << std::setw(3)
        << millis.count();

    return oss.str();
}
