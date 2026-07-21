#include <sys/socket.h>

#include "client_registry.hpp"


void ClientRegistry::add(int fd) {
    std::scoped_lock<std::mutex> lock(mtx_);
    clients_.insert(fd);
}

void ClientRegistry::remove(int fd) {
    std::scoped_lock lock(mtx_);
    clients_.erase(fd);
}

// shutdown(), не close(): будим чужой recv() в другом потоке, не отбирая
// сам fd — его закроет владеющий поток после join().
void ClientRegistry::shutdown_all() {
    std::scoped_lock lock(mtx_);
    for (int fd : clients_) {
        shutdown(fd, SHUT_RDWR);
    }
}
