#pragma once

#include <mutex>
#include <unordered_set>


class ClientRegistry {
public:
    void add(int fd);
    void remove(int fds);
    void shutdown_all();  // будит все recv(), блокирующиеся на активных client_fd

private:
    std::mutex mtx_;
    std::unordered_set<int> clients_;

};

