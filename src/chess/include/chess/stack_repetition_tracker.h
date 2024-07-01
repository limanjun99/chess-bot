#pragma once

#include <cstdint>

#include "board.h"
#include "constants.h"
#include "move.h"

namespace chess {

// This class helps to keep track of seen board positions to detect threefold repetition.
// This functionality is not provided in the Board class as it cannot be done efficiently.
// Note that this tracker must have positions added / removed in filo order
// (other orderings are not supported by this library).
class StackRepetitionTracker {
public:
  // Visit a new board position, which was reached from the given move (or null move if first position).
  constexpr void push(const Board& board, const Move& move = Move::null());

  // Unvisit the last board position.
  constexpr void pop();

  // Check if the last added board position is a draw.
  constexpr bool is_repetition_draw() const;

private:
  struct Position {
    Board::Hash hash;
    int repetition_count;
  };
  std::vector<Position> positions;
};

// ========== IMPLEMENTATIONS ==========

constexpr void StackRepetitionTracker::push(const Board& board, const Move& move) {
  if (!move.is_null() && (move.is_capture() || move.is_castle() || move.get_piece() == PieceType::Pawn)) {
    // Irreversible move. Insert two null hashes to indicate that earlier positions need
    // not be considered for draw by repetition.
    positions.push_back(Position{Board::Hash::null, 1});
    positions.push_back(Position{Board::Hash::null, 0});
  }

  const Board::Hash hash{board.get_hash()};
  int repetition_count{1};
  for (int i{static_cast<int>(positions.size()) - 2}; i >= 0; i -= 2) {
    if (positions[i].hash.is_null()) break;
    if (positions[i].hash != hash) continue;
    repetition_count = positions[i].repetition_count + 1;
    break;
  }

  positions.push_back(Position{hash, repetition_count});
}

constexpr void StackRepetitionTracker::pop() {
  positions.pop_back();
  while (!positions.empty() && positions.back().hash.is_null()) positions.pop_back();
}

constexpr bool StackRepetitionTracker::is_repetition_draw() const {
  if (positions.empty()) return false;
  return positions.back().repetition_count >= constants::threefold_repetition;
}

}  // namespace chess