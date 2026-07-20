#pragma once
#include <mutex>
#include <set>


class ClientRegistry {
public:
    void add(int fd);
    void remove(int fd);
    void shutdown_all();  // будит все recv(), блокирующиеся на активных client_fd

private:
    std::mutex mtx_;
    std::set<int> active_fds_;
};

