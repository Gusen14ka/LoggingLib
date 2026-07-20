

#include "timer.hpp"

#include <iostream>

#include "print_stats/print_stats.hpp"


TimerLoop::TimerLoop(Statistics &statistics,
    std::mutex &cout_mtx, size_t report_interval)
    : statistics_(statistics),
    cout_mtx_(cout_mtx),
    report_interval_(report_interval),
    thread_(&TimerLoop::run, this) {}

TimerLoop::~TimerLoop() {
    {
        std::scoped_lock lock(stop_mtx_);
        stopped_ = true;
    }

    stop_cond_.notify_one();
    thread_.join();
}

void TimerLoop::run() {
    Snapshot snapshot {};

    std::unique_lock<std::mutex> stop_lock(stop_mtx_);

    while (!stopped_) {
        stop_cond_.wait_for(
            stop_lock, std::chrono::seconds(report_interval_),
            [this]() { return stopped_; });

        if (stopped_) {
            break;
        }

        auto newSnapshot = statistics_.snapshot();
        if (snapshot != newSnapshot) {
            std::scoped_lock<std::mutex> lock(cout_mtx_);
            print_stats(newSnapshot, std::cout, "(time interval)");
            snapshot = newSnapshot;
        }
    }
}
