#pragma once

#include "evaluation.h"

// Configuration for the engine.
namespace config {
// Depth of quiescence search.
constexpr int quiescence_search_depth = 8;

// Safety value used in delta pruning during quiescence search. Ignores move if the evaluation + safety is below alpha.
constexpr Evaluation quiescence_search_delta_pruning_safety{500};

// The depth of subtree searched in null move heuristic is reduced by an additional R.
constexpr int null_move_heuristic_R = 2;

// Maximum depth the engine searches to.
constexpr int max_depth = 64;

// Size of transposition table. Roughly 4 million.
constexpr int transposition_table_size = 1 << 22;

// If the expected value of a move does not raise evaluation to within this amount of the alpha, then prune it.
constexpr Evaluation futility_margin{500};
}  // namespace config
