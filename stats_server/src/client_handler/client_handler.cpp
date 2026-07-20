#include <sys/socket.h>
#include <iostream>
#include <ostream>
#include <unistd.h>

#include "client_handler.hpp"
#include "logger/utils/error_utils.hpp"
#include "log_entry/log_entry.hpp"
#include "print_stats/print_stats.hpp"

namespace {
    constexpr size_t CHUNK_SIZE = 4096;
}

void handle_client(int client_fd, Statistics& statistics, std::mutex& cout_mtx,
                    size_t report_message_interval, ClientRegistry& registry) {
    registry.add(client_fd);

    std::string buffer;
    char chunk[CHUNK_SIZE];

    while (true) {
        ssize_t bytes_read = recv(client_fd, chunk, CHUNK_SIZE, 0);
        // -1 — ошибка recv
        if (bytes_read == -1) {
            std::scoped_lock<std::mutex> lock(cout_mtx);
            std::cerr << "recv failed: " << strerror_safe(errno) << std::endl;
            break;
        }

        // штатное закрытие — как обычным клиентом, так и через shutdown_all()
        if (bytes_read == 0) {
            break;
        }

        buffer.append(chunk, bytes_read);

        size_t sep;
        while ((sep = buffer.find('\n')) != std::string::npos) {
            auto line = buffer.substr(0, sep);
            auto entry = LogEntry::parse(line);

            buffer.erase(0, sep + 1);

            if (!entry) {
                std::scoped_lock<std::mutex> lock(cout_mtx);
                std::cerr << "Failed parsing log-message: " << line << std::endl;
                continue;
            }

            {
                std::scoped_lock<std::mutex> lock(cout_mtx);
                std::cout << line << std::endl;
            }

            if (statistics.update(*entry) % report_message_interval == 0) {
                std::scoped_lock<std::mutex> lock(cout_mtx);
                print_stats(statistics.snapshot(), std::cout, "(message count)");
            }
        }
    }
    registry.remove(client_fd);
    close(client_fd);
}