#pragma once

#include <chrono>

#include "chess/board.h"
#include "chess/move.h"
#include "chess/stack_repetition_tracker.h"
#include "config.h"
#include "history_heuristic.h"
#include "killer_moves.h"
#include "transposition_table.h"

class Engine {
public:
  using time_point = std::chrono::time_point<std::chrono::steady_clock>;

  Engine();
  Engine(const chess::Board& board);

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

  // Set the board position. This resets all data of the engine.
  void set_position(const chess::Board& board);

  // Apply the given move to the board.
  void apply_move(const chess::Move& move);

  // Choose a move to make for the current player of the given board within the given `search_time`.
  MoveInfo choose_move(std::chrono::milliseconds search_time);

  // Choose a move to make for the current player of the given board, searching at the given depth.
  MoveInfo choose_move(int search_depth);

private:
  chess::Board current_board;
  KillerMoves killer_moves;
  TranspositionTable transposition_table;
  HistoryHeuristic history_heuristic;
  chess::StackRepetitionTracker repetition_tracker;

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const chess::Board& board);

  // Evaluate the priority of the given move. Higher priority moves should be searched first.
  int evaluate_move_priority(const chess::Move& move, int depth_left, const chess::Move& hash_move, bool is_white);

  // Evaluate the priority of the given quiescence move. Higher priority moves should be searched first.
  int evaluate_quiescence_move_priority(const chess::Move& move);

  // Continue traversing the search tree. Returns the evaluation of the board for the board's current player.
  int search(const chess::Board& board, int alpha, int beta, int depth_left, DebugInfo& debug, bool& timed_out,
             time_point cutoff_time = time_point::max());

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player. Note that `depth_left` starts from 0 and decreases, so that all `depth_left`
  // in quiescence search is lower than in normal search.
  int quiescence_search(const chess::Board& board, int alpha, int beta, int depth_left, DebugInfo& debug);

  // Clears outdated information between each search depth in iterative deepening.
  void reset_iteration();

  // Clears outdated information between different board positions when choosing move.
  void reset_search();

  // Search at the given depth. Returns the best move if search completes before end_time, else returns Move::null().
  // A candidate best move can help guide the search (a good choice is the best move found in the previous iteration).
  chess::Move iterative_deepening(int search_depth, chess::Move candidate_best_move, DebugInfo& debug,
                                  time_point end_time = time_point::max());
};