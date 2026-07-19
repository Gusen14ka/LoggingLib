#pragma once

#include <memory>
#include <mutex>

#include "logger/strategies/log_strategy.hpp"


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