#include <iostream>

#include "cli/cli.hpp"
#include "logger/logger.hpp"
#include "log_worker/log_worker.hpp"

int main(int argc, char** argv) {
    auto config = std::move(parse_args(argc, argv));
    if (!config) {
        std::cerr << "Program stopped with error." << std::endl;
        return 1;
    }

    auto logger = Logger::create(config->log_file_name, config->default_level);
    if (!logger) {
        std::cerr << "Program stopped with error." << std::endl;
        return 1;
    }

    LogWorker worker = LogWorker(*logger);
    std::cout << "Logger console app started.\n"
          << "Enter a message to log. Optionally prefix it with a level:\n"
          << "<LEVEL> <message>   (e.g. \"ERROR Connection refused\")\n"
          << "<message>           (uses the default level)\n"
          << "Valid levels: DEBUG, INFO, WARNING, ERROR\n"
          << "Press Ctrl+D or enter an empty line to exit.\n";

    std::string message;
    while (true) {
        std::cout << "Enter log-message:" << std::endl;
        std::getline(std::cin, message);
        if (message.empty()) {
            return 0;
        }
        auto [log, log_level] = parse_log_level(message);
        worker.push(LogMessage{.message = std::move(log), .level = log_level});
    }
}
