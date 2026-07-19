

#include "log_worker.hpp"
#include "logger/logger.hpp"


LogWorker::LogWorker(Logger& logger) : logger_(logger), thread_(&LogWorker::run, this) {}

LogWorker::~LogWorker() {
    {
        std::scoped_lock<std::mutex> lock(mtx_);
        stop_ = true;
    }
    cv_.notify_one();
    thread_.join();
}

void LogWorker::run() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]() {
            return stop_ || !messages_.empty();
        });

        if (messages_.empty()) {
            return;
        }

        auto [message, level] = std::move(messages_.front());
        messages_.pop();

        if (level) {
            logger_.log(message, level.value());
        } else {
            logger_.log(message);
        }
    }
}

void LogWorker::push(LogMessage message) {
    std::scoped_lock<std::mutex> lock(mtx_);
    messages_.push(std::move(message));
    cv_.notify_one();
}
