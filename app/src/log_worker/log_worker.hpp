#pragma once
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

class Logger;
enum class LogLevel;

struct LogMessage {
    std::string message;
    std::optional<LogLevel> level;
};

class LogWorker {
    std::queue<LogMessage> messages_;
    Logger& logger_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_ = false;
    std::thread thread_;

public:
    LogWorker(Logger& logger);
    ~LogWorker();

    LogWorker(const LogWorker&) = delete;
    LogWorker& operator=(const LogWorker&) = delete;

    void push(LogMessage message);

private:
    void run();
};

