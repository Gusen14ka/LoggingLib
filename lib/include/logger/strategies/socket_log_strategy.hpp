#pragma once

#include <memory>
#include <mutex>

#include "logger/strategies/log_strategy.hpp"


// Отправляет записи логов по сетевому сокету (TCP). Не бросает исключений: в случае сетевой ошибки возвращает false
// и записывает описание ошибки в err. Потокобезопасна для одновременных вызовов write через внутренний mtx_.
// Создаётся через статический SocketLogStrategy::create(...); приватный конструктор обеспечивает корректную инициализацию.
class SocketLogStrategy : public ILogStrategy {
    int socket_fd_ = -1;
    std::mutex mtx_;
public:
    static std::unique_ptr<SocketLogStrategy> create(std::string const & host, int port, std::string& err);
    ~SocketLogStrategy() override;

    SocketLogStrategy(SocketLogStrategy const &) = delete;
    SocketLogStrategy& operator=(SocketLogStrategy const &) = delete;

    bool write(const std::string &message, std::string &err) override;
private:
    explicit SocketLogStrategy(int socket_fd);
};