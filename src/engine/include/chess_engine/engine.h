#pragma once

#include <chrono>
#include <map>

#include "chess/board.h"
#include "chess/move.h"
#include "config.h"
#include "killer_moves.h"
#include "repetition_table.h"
#include "transposition_table.h"

class Engine {
public:
  Engine();

  struct MoveInfo {
    Move move;
    std::chrono::milliseconds time_spent;  // Time in milliseconds spent searching.
    int search_depth;                      // Maximum depth reached during search.
    int normal_node_count;                 // Number of nodes in the main search tree.
    int quiescence_node_count;             // Number of nodes in quiescence search.
  };
  // Choose a move to make for the current player of the given board within the given `search_time`.
  MoveInfo choose_move(const Board& board, std::chrono::milliseconds search_time);

  // Choose a move to make for the current player of the given board, searching at the given depth.
  MoveInfo choose_move(const Board& board, int depth);

  // Add the given position to the set of seen positions (to avoid threefold repetition).
  void add_position(const Board& board);

private:
  int normal_node_count;
  int quiescence_node_count;
  KillerMoves killer_moves;
  TranspositionTable transposition_table;
  RepetitionTable repetition_table;

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const Board& board);

  // Evaluate the priority of the given move. Higher priority moves should be searched first.
  int evaluate_move_priority(const Move& move, int depth_left, const Move& refutation);

  // Evaluate the priority of the given quiescence move. Higher priority moves should be searched first.
  int evaluate_quiescence_move_priority(const Move& move);

  // Continue traversing the search tree. Returns the evaluation of the current board for the current player.
  int search(const Board& board, int alpha, int beta, int depth_left);

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player. Note that `depth_left` starts from 0 and decreases, so that all `depth_left`
  // in quiescence search is lower than in normal search.
  int quiescence_search(const Board& board, int alpha, int beta, int depth_left);

  // Clears outdated information between each search depth increase in iterative deepening.
  void soft_reset();
};