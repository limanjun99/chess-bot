#pragma once

#include <utility>
#include <vector>

#include "config.h"
#include "puzzle.h"
#include "statistic.h"

namespace data {

// Gets data from csv files and validates them. Throws a `runtime_error` if data is invalid.
std::vector<std::pair<Puzzle, Statistic>> get(const Config& config);

// Generate a new set of puzzles and corresponding (empty) statistics.
void generate(const Config& config);

// Write statistics to a new csv file.
void update_statistics(std::span<const Statistic> statistics, const Config& config);

}  // namespace data