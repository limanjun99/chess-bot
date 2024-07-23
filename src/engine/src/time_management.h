#pragma once

#include <chrono>

#include "chess/color.h"
#include "uci.h"

namespace time_management {

// Decide the maximum amount of time to spend searching on the next move.
// Returns std::nullopt if the search should be indefinite.
std::optional<std::chrono::milliseconds> decide(const engine::uci::SearchConfig& config, chess::Color player_color);

}  // namespace time_management