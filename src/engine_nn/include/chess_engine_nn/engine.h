#pragma once

#include <memory>

#include "chess/board.h"

class Engine {
public:
  Engine(const Board& board, Color me);
  Engine(Board&& board, Color me);

  // Update the board position with a new move.
  void apply_move(const Move& move);

  // Choose a move to make for the current player of the given board.
  Move choose_move();

  ~Engine();

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};