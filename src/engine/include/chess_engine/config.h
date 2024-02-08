#pragma once

// Configuration for the engine.
namespace config {
// When searching by iterative deepening, stop the search once this ratio of search time is reached.
constexpr double search_time_lowerbound = 0.99;

// Depth of quiescence search.
constexpr int quiescence_search_depth = 8;

// Safety value used in delta pruning during quiescence search. Ignores move if the evaluation + safety is below alpha.
constexpr int quiescence_search_delta_pruning_safety = 500;

// The depth of subtree searched in null move heuristic is reduced by an additional R.
constexpr int null_move_heuristic_R = 2;

// Maximum depth the engine searches to.
constexpr int max_depth = 64;

// Number of killer moves to store per ply.
constexpr int killer_move_count = 2;

// Size of transposition table. Roughly 16 million.
constexpr int transposition_table_size = 1 << 24;

// Size of repetition table. Roughly 16k, which is sufficient for longest possible chess game.
constexpr int repetition_table_size = 1 << 14;

// Check for timeout every time this amount of nodes have been searched.
constexpr int timeout_check_interval = 8192;

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

// Scores for move priority.
namespace move_priority {
constexpr int hash_move = 1e6;

constexpr int capture = 4e5;

// Ordered by most valuable victim, then least valuable attacker. Indexed by [victim][attacker].
constexpr u64 mvv_lva[7][6] = {
    {33, 30, 34, 35, 31, 32},  // Bishop victim.
    {0, 0, 0, 0, 0, 0},        // King victim (just filler as king can't be captured).
    {23, 20, 24, 25, 21, 22},  // Knight victim.
    {13, 10, 14, 15, 11, 12},  // Pawn victim.
    {53, 50, 54, 55, 51, 52},  // Queen victim.
    {43, 40, 44, 45, 41, 42},  // Rook victim.
    {0, 0, 0, 0, 0, 0},        // None victim (i.e. not a capture).
};

constexpr u64 promotion = 3e5;
constexpr int promotion_piece[6] = {2, 0, 1, 0, 4, 3};

constexpr int killer = 2e5;
constexpr int killer_index = 1;  // To prioritise more recent killer moves.

// History heuristic gives a evaluation of at most this value.
constexpr int history_heuristic_scale = 1e5;
}  // namespace move_priority