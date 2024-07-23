#pragma once

// Configuration for the engine.
namespace config {
// Depth of quiescence search.
constexpr int quiescence_search_depth = 8;

// Safety value used in delta pruning during quiescence search. Ignores move if the evaluation + safety is below alpha.
constexpr int quiescence_search_delta_pruning_safety = 500;

// The depth of subtree searched in null move heuristic is reduced by an additional R.
constexpr int null_move_heuristic_R = 2;

// Maximum depth the engine searches to.
constexpr int max_depth = 64;

// Size of transposition table. Roughly 16 million.
constexpr int transposition_table_size = 1 << 24;

// If the expected value of a move does not raise evaluation to within this amount of the alpha, then prune it.
constexpr int futility_margin = 500;
}  // namespace config

// Evaluation of the board (is in centipawns).
namespace evaluation {
// The scores have been chosen to fit within int16_t, so that they can be stored in the transposition table.
constexpr int min = -31'000;

constexpr int losing = -10'000;

constexpr int draw = 0;

constexpr int winning = 10'000;

constexpr int max = 31'000;

constexpr int piece[6] = {300, 10000, 300, 100, 900, 500};
}  // namespace evaluation
