#pragma once

#include "statistics/statistics.hpp"

void handle_client(int client_fd, Statistics& statistics, std::mutex& cout_mtx);
