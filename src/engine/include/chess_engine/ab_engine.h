#pragma once
#include "engine.h"

// An engine that uses Alpha Beta pruning to speedup its search.
class AlphaBetaEngine : public Engine {
public:
  AlphaBetaEngine(int max_depth);

  // Choose a move to make for the current player of the given board.
  Engine::MoveInfo make_move(const Board& board) override;

  ~AlphaBetaEngine() override = default;

private:
  int max_depth;
  int normal_node_count;
  int quiescence_node_count;

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const Board& board);

  // Evaluate the priority of the given move. Higher priority moves should be searched first.
  int evaluate_move_priority(const Move& move, const Board& board);

  // Continue traversing the search tree. Returns the evaluation of the current board for the current player.
  int search(const Board& board, int alpha, int beta, int current_depth);

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player.
  int quiescence_search(const Board& board, int alpha, int beta, int current_depth);
};