#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <string>

#include <logger/logger.hpp>

namespace {

constexpr LogLevel kAllLevels[] = {
    LogLevel::DEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR
};

constexpr char const* kSampleMessages[] = {
    "User logged in",
    "Cache miss for key",
    "Connection pool exhausted",
    "Retrying request",
    "Background job completed",
    "Configuration reloaded",
};

// Один "виртуальный клиент" — свой Logger с сокет-backend, отправляет messages_count
// сообщений со случайным уровнем/текстом и случайной небольшой задержкой между ними —
// имитирует несколько независимых источников логов, пишущих в один сервер статистики.
void simulate_client(int client_id, std::string const& host, int port,
                      size_t messages_count, int delay_ms) {
    auto logger = Logger::create(host, port, LogLevel::DEBUG);
    if (!logger) {
        std::cerr << "[client " << client_id << "] failed to connect to " << host << ":" << port << std::endl;
        return;
    }

    std::mt19937 rng(std::random_device{}() + client_id);
    std::uniform_int_distribution<size_t> level_dist(0, std::size(kAllLevels) - 1);
    std::uniform_int_distribution<size_t> message_dist(0, std::size(kSampleMessages) - 1);

    for (size_t i = 0; i < messages_count; i++) {
        auto level = kAllLevels[level_dist(rng)];
        std::string text = std::string(kSampleMessages[message_dist(rng)])
                          + " (client " + std::to_string(client_id) + ", #" + std::to_string(i) + ")";

        logger->log(text, level);

        if (delay_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    }

    std::cout << "[client " << client_id << "] finished sending " << messages_count << " messages" << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0]
                   << " <host> <port> <num_clients> <messages_per_client> <delay_ms>" << std::endl;
        std::cerr << "  delay_ms — пауза между сообщениями каждого клиента (0 — без паузы, "
                     "полезно для проверки триггера по N)" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);           // демо-инструмент, не продакшн-код — stoi тут допустим
    int num_clients = std::stoi(argv[3]);
    int messages_per_client = std::stoi(argv[4]);
    int delay_ms = std::stoi(argv[5]);

    std::vector<std::thread> clients;
    for (int i = 0; i < num_clients; i++) {
        clients.emplace_back(simulate_client, i, host, port,
                              static_cast<size_t>(messages_per_client), delay_ms);
    }

    for (auto& t : clients) {
        t.join();
    }

    std::cout << "All clients finished." << std::endl;
    return 0;
}