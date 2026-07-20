#pragma once

#include <condition_variable>
#include <thread>

#include "statistics/statistics.hpp"


class TimerLoop {
    Statistics& statistics_;
    std::mutex& cout_mtx_;
    size_t report_interval_;

    std::mutex stop_mtx_;
    bool stopped_ = false;
    std::condition_variable stop_cond_;

    std::thread thread_;

public:
    TimerLoop(Statistics& statistics, std::mutex& cout_mtx, size_t report_interval);
    ~TimerLoop();

    TimerLoop(TimerLoop const&) = delete;
    TimerLoop& operator=(TimerLoop const&) = delete;

private:
    void run();
};