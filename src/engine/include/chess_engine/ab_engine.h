#pragma once
#include "engine.h"

// An engine that uses Alpha Beta pruning to speedup its search.
class AlphaBetaEngine : public Engine {
public:
  AlphaBetaEngine(int depth = 5);

  // Choose a move to make for the current player of the given board.
  Move make_move(const Board& board) override;

  ~AlphaBetaEngine() override = default;

private:
  int depth;

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const Board& board);

  // Continue traversing the search tree. Returns the evaluation of the current board for the current player.
  int search(const Board& board, int alpha, int beta, int current_depth);

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player.
  int quiescence_search(const Board& board, int alpha, int beta, int current_depth);
};