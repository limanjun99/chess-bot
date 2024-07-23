#pragma once

#include "chess/move.h"
#include "config.h"

// Killer moves are stored at each depth by order of recency (most recent has smallest index).
class KillerMoves {
public:
  // Number of killer moves to store per ply.
  static constexpr int count{2};

  // Add the killer move at the given `depth_left`.
  void add(const chess::Move& move, int depth_left);

  // Clear all stored killer moves.
  void clear();

  // Returns the killer move at the given `depth_left` and index.
  const chess::Move& get(int depth_left, int index) const;

private:
  chess::Move killer_moves[config::max_depth][KillerMoves::count];
};