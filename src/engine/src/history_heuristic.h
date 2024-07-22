#pragma once

#include <cstdint>

#include "chess/bitboard.h"

class HistoryHeuristic {
public:
  HistoryHeuristic();

  // Add a move that caused a beta-cutoff.
  void add_move_success(bool is_white, chess::Bitboard from, chess::Bitboard to);

  // Add a move that failed to cause a beta-cutoff.
  void add_move_failure(bool is_white, chess::Bitboard from, chess::Bitboard to);

  // Reset scores of all moves.
  void clear();

  // Returns the move priority scoring of the given move.
  int get_score(bool is_white, chess::Bitboard from, chess::Bitboard to);

private:
  uint64_t successes[2][64][64];
  uint64_t totals[2][64][64];
};