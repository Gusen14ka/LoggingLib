#include <sys/socket.h>

#include "client_registry.hpp"


void ClientRegistry::add(int fd) {
    std::scoped_lock<std::mutex> lock(mtx_);
    active_fds_.insert(fd);
}

void ClientRegistry::remove(int fd) {
    std::scoped_lock lock(mtx_);
    active_fds_.erase(fd);
}

void ClientRegistry::shutdown_all() {
    std::scoped_lock lock(mtx_);
    for (int fd : active_fds_) {
        shutdown(fd, SHUT_RDWR);
    }
}
