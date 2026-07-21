#include <cstring>
#include <string>

#include "logger/strategies/file_log_strategy.hpp"
#include "logger/utils/error_utils.hpp"


std::unique_ptr<FileLogStrategy> FileLogStrategy::create(std::string const& log_file_name, std::string& err) {
    std::ofstream log_file(log_file_name);
    if (!log_file.is_open()) {
        err = strerror_safe(errno);
        return nullptr;
    }
    return std::unique_ptr<FileLogStrategy>(new FileLogStrategy(std::move(log_file)));
}

FileLogStrategy::FileLogStrategy(std::ofstream log_file) : log_file_(std::move(log_file)) {}

FileLogStrategy::~FileLogStrategy() {
    log_file_.close();
}

bool FileLogStrategy::write(std::string const &message, std::string &err) {
    std::scoped_lock<std::mutex> lock(mtx_);
    log_file_ << message;
    if (!is_stream_alive()) {
        err = strerror_safe(errno);
        return false;
    }
    return true;
}

bool FileLogStrategy::is_stream_alive() const {
    return log_file_.is_open() && log_file_.good();
}
