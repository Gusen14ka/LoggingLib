#pragma once

#include "statistics/statistics.hpp"
#include <client_registry/client_registry.hpp>

void handle_client(int client_fd, Statistics& statistics, std::mutex& cout_mtx,
                    size_t report_message_interval, ClientRegistry& registry);
