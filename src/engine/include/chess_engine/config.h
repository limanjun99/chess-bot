#pragma once

// Configuration for the engine.
namespace config {
// When searching by iterative deepening, stop the search once this ratio of search time is reached.
constexpr double search_time_lowerbound = 0.9;

// Depth of quiescence search.
constexpr int quiescence_search_depth = 4;

// Include checks in the search when within this depth of the quiescence search.
constexpr int quiescence_search_check_depth = 2;

// The depth of subtree searched in null move heuristic is reduced by an additional R.
constexpr int null_move_heuristic_R = 2;
}  // namespace config

// Evaluation of the board (is in centipawns).
namespace evaluation {
constexpr int LOSING = -1000000000;
constexpr int WINNING = 1000000000;
constexpr int DRAW = 0;
constexpr int piece[6] = {300, 10000, 300, 100, 900, 500};
}  // namespace evaluation