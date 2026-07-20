#include <iostream>
#include <mutex>
#include <ostream>
#include <thread>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <csignal>

#include "log_entry/log_entry.hpp"
#include "logger/utils/error_utils.hpp"
#include "cli/cli.hpp"
#include "client_handler/client_handler.hpp"
#include "timer/timer.hpp"

namespace {
    volatile sig_atomic_t g_running = 1;

    void handle_signal(int) {
        g_running = 0;
    }
}

int main(int argc, char** argv) {
    // tzset() инициализирует глобальное состояние таймзоны (переменные timezone/tzname
    // и т.д.), которое использует mktime(). Вызываем один раз здесь, в main(),
    // ДО запуска client-потоков, так как mktime() не полностью потокобезопасна по стандарту,
    // и если бы каждый поток самостоятельно триггерил ленивую инициализацию таймзоны
    // при первом вызове mktime(), несколько потоков могли бы одновременно писать
    // в это глобальное состояние. Явный вызов здесь устраняет саму возможность гонки:
    // к моменту старта потоков состояние уже проинициализировано и дальше только читается.
    tzset();

    // Регистрация обработчика сигнала для graceful shutdown
    struct sigaction sa {};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);


    auto config = parse_args(argc, argv);
    if (!config) {
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(config->port);

    if (inet_pton(AF_INET, config->host.c_str(), &server_address.sin_addr) != 1) {
        std::cerr << "Invalid host address: '" << config->host << "'" << std::endl;
        return 1;
    }

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&server_address),
        sizeof(server_address)) == -1) {
        std::cerr << "Error creating bind" << std::endl;
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        std::cerr << "Error creating listen" << std::endl;
        return 1;
    }

    Statistics statistics;
    std::mutex cout_mtx;
    ClientRegistry registry;

    // Запуск таймера в отдельном потоке
    // Важно, что таймер иниц. после statistics и cout_mtx
    // => его конструктор будет вызван раньше и ресурсы безопасно освободятся
    auto timer = TimerLoop(statistics, cout_mtx, config->report_interval);

    // Храним потоки, чтобы поток закрыть
    std::vector<std::thread> client_threads;

    while (g_running) {
        struct sockaddr_in client_address {};
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(server_fd,
            reinterpret_cast<sockaddr*>(&client_address), &client_len);

        if (client_fd == -1) {
            if (errno == EINTR && !g_running)
                // Сигнал завершения процесса
                break;
            std::scoped_lock<std::mutex> lock(cout_mtx);
            std::cerr << "accept failed: " << strerror_safe(errno) << std::endl;
            continue;  // не фатально — пробуем принять следующего клиента
        }

        client_threads.emplace_back(handle_client, client_fd,
            std::ref(statistics), std::ref(cout_mtx),
            config->report_messages_interval, std::ref(registry));

    }

    // Отправляем сигнал о завершении,
    // из-за которого recv разблокируется и вернёт 0
    registry.shutdown_all();
    for (auto& t : client_threads) {
        t.join();
    }
    close(server_fd);
    return 0;
}
