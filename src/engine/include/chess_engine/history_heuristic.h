#pragma once

#include "chess/bitboard.h"

class HistoryHeuristic {
public:
  HistoryHeuristic();

  // Add a move that caused a beta-cutoff.
  void add_move_success(bool is_white, u64 from, u64 to);

  // Add a move that failed to cause a beta-cutoff.
  void add_move_failure(bool is_white, u64 from, u64 to);

  // Reset scores of all moves.
  void clear();

  // Returns the move priority scoring of the given move.
  int get_score(bool is_white, u64 from, u64 to);

private:
  u64 successes[2][64][64];
  u64 totals[2][64][64];
};