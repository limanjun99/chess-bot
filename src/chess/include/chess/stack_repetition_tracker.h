#pragma once

#include <cstdint>

#include "board.h"
#include "move.h"

namespace chess {

// This class helps to keep track of seen board positions to detect threefold repetition.
// This functionality is not provided in the Board class as it cannot be done efficiently.
// Note that this tracker must have positions added / removed in fifo order
// (other orderings are not supported by this library).
class StackRepetitionTracker {
public:
  // Visit a new board position, which was reached from the given move (or null move if first position).
  void push(const Board& board, const Move& move = Move::null());

  // Unvisit the last board position.
  void pop();

  // Check if the last added board position is a draw.
  bool is_repetition_draw() const;

private:
  struct Position {
    Board::Hash hash;
    int repetition_count;
  };
  std::vector<Position> positions;
};

}  // namespace chess