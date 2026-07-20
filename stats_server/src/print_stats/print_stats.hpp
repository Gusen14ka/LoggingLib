#pragma once

#include <ostream>
#include "statistics/statistics.hpp"

void print_stats(Snapshot const& snap, std::ostream& out, std::string_view cause);
