#pragma once
#include <fstream>
#include <mutex>

enum class MessageLevel {DEBUG, INFO, WARNING, ERROR};

class Logger {
    MessageLevel default_level_;
    std::ofstream log_file_;
    std::mutex mtx_;

    public:
        Logger(std::string const & log_file_name, MessageLevel default_level);
        ~Logger();
        void log(std::string const & message, MessageLevel level);
        void log(std::string const & message);
        void change_default_level(MessageLevel level);

        // TODO: Дописать все 6 к-тор и опер-ов
        Logger(Logger const & o) = delete;
        Logger& operator=(Logger const & o) = delete;

    private:
        std::string current_timestamp();
        bool is_valid(std::string& err) const;
};