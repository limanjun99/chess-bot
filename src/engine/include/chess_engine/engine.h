#pragma once

#include <chrono>
#include <map>

#include "chess/board.h"
#include "chess/move.h"
#include "config.h"
#include "history_heuristic.h"
#include "killer_moves.h"
#include "repetition_table.h"
#include "transposition_table.h"

class Engine {
public:
  Engine();

  struct DebugInfo {
    int evaluation;                   // Evaluation of the board position after playing the chosen move.
    int normal_node_count;            // Number of nodes in the main search tree.
    int quiescence_node_count;        // Number of nodes in quiescence search.
    int null_move_success;            // Nodes that returned after a null move search.
    int null_move_total;              // Nodes that did a null move search.
    int transposition_table_success;  // Nodes that returned immediately after checking transposition table.
    int transposition_table_total;    // Nodes that checked the transposition table.
    int q_delta_pruning_success;      // Nodes that were delta pruned.
    int q_delta_pruning_total;        // Nodes that tried to delta prune.
  };
  struct MoveInfo {
    chess::Move move;
    std::chrono::milliseconds time_spent;  // Time in milliseconds spent searching.
    int search_depth;                      // Maximum depth reached during search.
    DebugInfo debug;
  };
  // Choose a move to make for the current player of the given board within the given `search_time`.
  MoveInfo choose_move(const chess::Board& board, std::chrono::milliseconds search_time);

  // Choose a move to make for the current player of the given board, searching at the given depth.
  MoveInfo choose_move(const chess::Board& board, int depth);

  // Add the given position to the set of seen positions (to avoid threefold repetition).
  void add_position(const chess::Board& board);

private:
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> cutoff_time;
  DebugInfo debug;
  KillerMoves killer_moves;
  TranspositionTable transposition_table;
  RepetitionTable repetition_table;
  HistoryHeuristic history_heuristic;
  bool search_timeout;

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const chess::Board& board);

  // Evaluate the priority of the given move. Higher priority moves should be searched first.
  int evaluate_move_priority(const chess::Move& move, int depth_left, const chess::Move& hash_move, bool is_white);

  // Evaluate the priority of the given quiescence move. Higher priority moves should be searched first.
  int evaluate_quiescence_move_priority(const chess::Move& move);

  // Continue traversing the search tree. Returns the evaluation of the current board for the current player.
  template <bool IsTimed>
  int search(const chess::Board& board, int alpha, int beta, int depth_left);

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player. Note that `depth_left` starts from 0 and decreases, so that all `depth_left`
  // in quiescence search is lower than in normal search.
  int quiescence_search(const chess::Board& board, int alpha, int beta, int depth_left);

  // Clears outdated information between each search depth increase in iterative deepening.
  void soft_reset();

  // Reset debug information between calls to `choose_move`.
  void reset_debug();

  // Check if the engine has ran out of time to search.
  bool is_out_of_time();
};