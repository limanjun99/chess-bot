#pragma once

#include <span>

#include "puzzle.h"
#include "statistic.h"

namespace solve {

// Solves the puzzle and returns statistics for it.
Statistic puzzle(const Puzzle& puzzle);

// Compares a new statistic against the old one, and print the changes.
void compare(const Statistic& new_statistic, const Statistic& old_statistics);

// Compares all the new statistics against the old ones, and print the total changes.
void compare_all(std::span<const Statistic> new_statistic, std::span<const Statistic> old_statistics);

}  // namespace solve