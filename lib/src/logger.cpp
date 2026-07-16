#include "logger/logger.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

std::string to_string(MessageLevel level){
    switch (level) {
        case MessageLevel::DEBUG: return "DEBUG";
        case MessageLevel::INFO: return "INFO";
        case MessageLevel::WARNING: return "WARNING";
        case MessageLevel::ERROR: return "ERROR";
    }

    return "UNKNOWN";
}

Logger::Logger(std::string const & log_file_name, MessageLevel default_level) : default_level_(default_level) {
    log_file_.open(log_file_name);
    std::string err;
    if (!is_valid(err)){
        std::cerr << "Something wrong with file: " + err << std::endl;
    }
}

Logger::~Logger(){
    log_file_.close();
}

void Logger::log(std::string const & message, MessageLevel level){
    if (level < default_level_) return;
    std::scoped_lock<std::mutex> lock(mtx_);
    std::string err;
    if (!is_valid(err)){
        std::cerr << "Something wrong with file: " + err << std::endl;
        return;
    }
    log_file_ << "["
        << to_string(level)
        << "] ["
        << current_timestamp()
        << "] ["
        << message
        << "]\n";
}

void Logger::log(std::string const & message){
    log(message, default_level_);
}

void Logger::change_default_level(MessageLevel level) {
    default_level_ = level;
}

bool Logger::is_valid(std::string& err) const{
    if (!log_file_.is_open()){
        err = "file is not open";
        return false;
    }
    if (log_file_.bad()){
        err = "irrecoverable I/O error (badbit)";
        return false;
    }
    if (log_file_.fail()){
        err = "logical error on stream (failbit)";
        return false;
    }
    return true;
}

std::string Logger::current_timestamp(){
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_buf {};
    #ifdef _WIN32
        localtime_s(&tm_buf, &time);
    #else
        localtime_r(&time, &tm_buf);
    #endif

    auto millis = 
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
