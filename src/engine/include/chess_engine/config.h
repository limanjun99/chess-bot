#pragma once

#include <array>
#include <cstdint>

#include "chess/piece.h"
#include "chess/pieces/base_piece.h"

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
constexpr int timeout_check_interval = 1 << 16;  // Roughly 50k.

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
constexpr std::array<std::array<uint64_t, 6>, 7> mvv_lva = []() {
  std::array<std::array<uint64_t, 6>, 7> mvv_lva{};
  const size_t bishop{static_cast<size_t>(chess::PieceType::Bishop)};
  const size_t king{static_cast<size_t>(chess::PieceType::King)};
  const size_t knight{static_cast<size_t>(chess::PieceType::Knight)};
  const size_t pawn{static_cast<size_t>(chess::PieceType::Pawn)};
  const size_t queen{static_cast<size_t>(chess::PieceType::Queen)};
  const size_t rook{static_cast<size_t>(chess::PieceType::Rook)};
  const size_t none{static_cast<size_t>(chess::PieceType::None)};
  const std::array<uint64_t, 6> piece_by_value{king, queen, rook, bishop, knight, pawn};

  mvv_lva[king] = {0, 0, 0, 0, 0, 0};  // King victim (just filler as king can't be captured).
  mvv_lva[none] = {0, 0, 0, 0, 0, 0};  // None victim (i.e. not a capture).
  uint64_t base{50};
  for (const auto victim : piece_by_value) {
    if (victim == king) continue;
    uint64_t increment{0};
    for (const auto attacker : piece_by_value) {
      mvv_lva[victim][attacker] = base + increment;
      increment++;
    }
    base -= 10;
  }

  return mvv_lva;
}();

constexpr uint64_t promotion = 3e5;
constexpr std::array<int, 6> promotion_piece = []() {
  std::array<int, 6> promotion_piece{};
  promotion_piece[static_cast<size_t>(chess::PieceType::Knight)] = 1;
  promotion_piece[static_cast<size_t>(chess::PieceType::Bishop)] = 2;
  promotion_piece[static_cast<size_t>(chess::PieceType::Rook)] = 3;
  promotion_piece[static_cast<size_t>(chess::PieceType::Queen)] = 4;
  return promotion_piece;
}();

constexpr int killer = 2e5;
constexpr int killer_index = 1;  // To prioritise more recent killer moves.

// History heuristic gives a evaluation of at most this value.
constexpr int history_heuristic_scale = 1e5;
}  // namespace move_priority