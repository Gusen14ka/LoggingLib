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
#include <poll.h>
#include <fcntl.h>

#include "log_entry/log_entry.hpp"
#include "logger/utils/error_utils.hpp"
#include "cli/cli.hpp"
#include "client_handler/client_handler.hpp"
#include "client_registry/client_registry.hpp"
#include "timer/timer.hpp"

namespace {
int g_shutdown_pipe[2] = {-1, -1};

void handle_signal(int) {
    char byte = 1;
    write(g_shutdown_pipe[1], &byte, 1);
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

    // Фиксируем self-pipe для сигнала остановки
    if (pipe(g_shutdown_pipe) == -1) {
        std::cerr << "Error creating pipe: " << strerror_safe(errno) << std::endl;
        return 1;
    }

    // Регистрация обработчика сигнала для graceful shutdown
    struct sigaction sa {};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    // Фиксируем self-pipe для завершения клиентский потоков
    int completion_pipe[2];
    if (pipe(completion_pipe) == -1) {
        std::cerr << "Error creating pipe: " << strerror_safe(errno) << std::endl;
        return 1;
    }

    // Делаем поток неблокирующим (при отсутсвии данных будет возврщать 0)
    int flags = fcntl(completion_pipe[0], F_GETFL, 0);
    fcntl(completion_pipe[0], F_SETFL, flags | O_NONBLOCK);


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

    // Храним потоки клиентских обработчиков, чтобы поток закрыть
    // fd -> поток, обслуживающий этого клиента
    std::unordered_map<int, std::thread> client_threads;

    // Подготавливаем ожидаемые события для poll
    std::vector<pollfd> fds = {
        {server_fd, POLLIN, 0},
        {g_shutdown_pipe[0], POLLIN, 0},
        {completion_pipe[0], POLLIN, 0},
    };

    bool shutting_down = false;

    while (true) {
        // for (auto& pfd : fds) pfd.revents = 0; // Зачем?

        // Бесконечно ожидаем собитие
        int ready = poll(fds.data(), fds.size(), -1);
        if (ready == -1) {
            if (errno == EINTR) continue;
            std::cerr << "Error polling: " << strerror_safe(errno) << std::endl;
            break;
        }

        // Получен сигнал завершения
        if (fds[1].revents & POLLIN) {
            char buf[64];
            read(g_shutdown_pipe[0], buf, sizeof(buf));
            shutting_down = true;
            registry.shutdown_all();
        }

        // Получен сигнал на join клиентского потока
        if (fds[2].revents & POLLIN) {
            int finished_id;
            // Читаем в цикле, так как могли успеть написать сразу несколько потоков
            while (read(completion_pipe[0], &finished_id,
                sizeof(finished_id)) == sizeof(finished_id)) {
                auto it = client_threads.find(finished_id);
                if (it != client_threads.end()) {
                    if (it->second.joinable()) {
                        it->second.join();
                    }
                    client_threads.erase(it);
                }
                close(finished_id);
            }
        }

        // Получен сигнал на accept нового клиента
        if (!shutting_down && (fds[0].revents & POLLIN)) {
            struct sockaddr_in client_address {};
            socklen_t client_len = sizeof(client_address);

            int client_fd = accept(server_fd,
                reinterpret_cast<sockaddr*>(&client_address), &client_len);

            if (client_fd == -1) {
                std::scoped_lock<std::mutex> lock(cout_mtx);
                std::cerr << "accept failed: " << strerror_safe(errno) << std::endl;
            } else {
                client_threads.emplace(client_fd,
                    std::thread(handle_client, client_fd,
                        std::ref(statistics), std::ref(cout_mtx),
                        config->report_messages_interval,
                        std::ref(registry), completion_pipe[1]));
            }
        }

        // Если был shutdown и все потоки joined - выходим
        if (shutting_down && client_threads.empty()) {
            break;
        }
    }

    // Закрываем серверный сокет и вспомогательные дискрипторы
    close(server_fd);
    close(g_shutdown_pipe[0]);
    close(g_shutdown_pipe[1]);
    close(completion_pipe[0]);
    close(completion_pipe[1]);

    return 0;
}
