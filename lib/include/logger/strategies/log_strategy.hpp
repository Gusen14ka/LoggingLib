#pragma once

#include <string>

class ILogStrategy {
public:
    virtual ~ILogStrategy() = default;
    virtual bool write(std::string const & message, std::string& err) = 0;
};
