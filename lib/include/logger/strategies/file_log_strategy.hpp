#pragma once

#include <fstream>
#include <memory>
#include <mutex>

#include "logger/strategies/log_strategy.hpp"

// Логирует текстовые сообщения в файл. Не бросает исключений: при ошибках записи возвращает false и передаёт описание ошибки через параметр err.
// Потокобезопасна: внутренняя синхронизация через mtx_.
// Создаётся через статический FileLogStrategy::create(...); конструктор приватный — это гарантирует,
// что успешно созданный объект всегда полностью работоспособен.
class FileLogStrategy : public ILogStrategy {
    std::ofstream log_file_;
    std::mutex mtx_;
public:
    static std::unique_ptr<FileLogStrategy> create(std::string const& log_file_name, std::string& err);
    ~FileLogStrategy() override;
    bool write(std::string const & message, std::string& err) override;
private:
    explicit FileLogStrategy(std::ofstream log_file);
    bool is_stream_alive() const;
};


