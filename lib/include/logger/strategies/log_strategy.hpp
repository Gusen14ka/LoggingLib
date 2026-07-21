#pragma once

#include <string>

// Интерфейс стратегии записи логов. Определяет единственный метод write(...).
// Реализации должны стремиться не бросать исключений: в случае ошибки возвращать false
// и заполнять параметр err коротким описанием причины.
class ILogStrategy {
public:
    virtual ~ILogStrategy() = default;
    virtual bool write(std::string const & message, std::string& err) = 0;
};
