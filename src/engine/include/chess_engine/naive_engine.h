#pragma once
#include "engine.h"

class NaiveEngine : public Engine {
public:
  NaiveEngine(int depth = 5);

  // Choose a move to make for the current player of the given board.
  Move make_move(const Board& board) override;

  ~NaiveEngine() override = default;

private:
  int depth;

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const Board& board);

  // Continue traversing the search tree. Returns the evaluation of the current board for the current player.
  int search(const Board& board, int current_depth);

  // Traverse the search tree until a position with no captures. Returns the evaluation of the current board for the
  // current player.
  int quiescence_search(const Board& board, int current_depth);
};