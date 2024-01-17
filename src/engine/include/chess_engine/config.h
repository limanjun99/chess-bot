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

// Maximum depth the engine searches to.
constexpr int max_depth = 64;

// Number of killer moves to store per ply.
constexpr int killer_move_count = 2;

// Size of transposition table. Roughly 16 million.
constexpr int transposition_table_size = 1 << 24;

// Size of repetition table. Roughly 16k, which is sufficient for longest possible chess game.
constexpr int repetition_table_size = 1 << 14;
}  // namespace config

// Evaluation of the board (is in centipawns).
namespace evaluation {
constexpr int min = -1'000'000'000;

constexpr int losing = -1'000'000;

constexpr int draw = 0;

constexpr int winning = 1'000'000;

constexpr int max = 1'000'000'000;

constexpr int piece[6] = {300, 10000, 300, 100, 900, 500};
}  // namespace evaluation

// Scores for move priority.
namespace move_priority {
constexpr int refutation = 1'000;

constexpr int capture = 100;

// Ordered by most valuable victim, then least valuable attacker. Indexed by [victim][attacker].
constexpr int mvv_lva[7][6] = {
    {33, 30, 34, 35, 31, 32},  // Bishop victim.
    {0, 0, 0, 0, 0, 0},        // King victim (just filler as king can't be captured).
    {23, 20, 24, 25, 21, 22},  // Knight victim.
    {13, 10, 14, 15, 11, 12},  // Pawn victim.
    {53, 50, 54, 55, 51, 52},  // Queen victim.
    {43, 40, 44, 45, 41, 42},  // Rook victim.
    {0, 0, 0, 0, 0, 0},        // None victim (i.e. not a capture).
};

// Killer moves have slightly lower priority than captures, as captures also add mvv_lva.
constexpr int killer = 100;
constexpr int killer_index = 1;  // To prioritise more recent killer moves.
}  // namespace move_priority