#pragma once

#include "chess/board.h"
#include "chess/move.h"

class Engine {
public:
  // Choose a move to make for the current player of the given board.
  virtual Move make_move(const Board& board) = 0;

  virtual ~Engine() = default;
};