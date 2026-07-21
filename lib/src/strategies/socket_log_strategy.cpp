#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>

#include "logger/strategies/socket_log_strategy.hpp"
#include "logger/utils/error_utils.hpp"


std::unique_ptr<SocketLogStrategy> SocketLogStrategy::create(std::string const & host, int port, std::string &err) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        err = strerror_safe(errno);
        return nullptr;
    }

    struct sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr.s_addr) == -1) {
        err = "invalid address: " + host; // inet_pton не устанавливает errno, поэтому пишем свой текст ошибки
        close(socket_fd);
        return nullptr;
    }

    if (connect(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
        err = strerror_safe(errno);
        close(socket_fd);
        return nullptr;
    }

    return std::unique_ptr<SocketLogStrategy>(new SocketLogStrategy(socket_fd));
}

SocketLogStrategy::SocketLogStrategy(int socket_fd) : socket_fd_(socket_fd) {}

SocketLogStrategy::~SocketLogStrategy() {
    if (socket_fd_ != -1) {
        close(socket_fd_);
    }
}

bool SocketLogStrategy::write(std::string const &message, std::string &err) {
    std::scoped_lock<std::mutex> lock(mtx_);

    size_t len = message.size();
    size_t written_bytes = 0;
    char const * data = message.c_str();
    while (written_bytes < len) {
        // MSG_NOSIGNAL предотвращает SIGPIPE, который иначе может убить весь процесс,
        // если удалённая сторона (сервер статистики) неожиданно закрыла соединение —
        // вместо сигнала send() штатно вернёт -1 с errno == EPIPE, что мы уже
        // обрабатываем как обычную ошибку записи.
        ssize_t n = send(socket_fd_, data + written_bytes, len - written_bytes, MSG_NOSIGNAL);
        if (n == -1) {
            err = strerror_safe(errno);
            return false;
        }
        written_bytes += static_cast<size_t>(n);
    }
    return true;
}
