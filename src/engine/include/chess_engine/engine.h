#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <thread>

#include "chess/board.h"
#include "chess/move.h"
#include "chess/stack_repetition_tracker.h"
#include "config.h"
#include "history_heuristic.h"
#include "killer_moves.h"
#include "transposition_table.h"
#include "uci.h"

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

  // Set the board position based on applying the `moves` to the `board`.
  void set_position(const chess::Board& board, std::span<chess::Move const> moves);

  // Clears all data about the current game.
  void reset();

  // Apply the given move to the board.
  void apply_move(const chess::Move& move);

  // Choose a move to make for the current player of the given board within the given `search_time`.
  MoveInfo choose_move(std::chrono::milliseconds search_time);

  // Choose a move to make for the current player of the given board, searching at the given depth.
  MoveInfo choose_move(int search_depth);

  // Performs a search with a configurations based on the uci go command.
  // The search is cancellable through the returned SearchControl object.
  // If `movetime` is provided, the search will highly likely terminate 1~2 ms before the movetime is used up.
  class SearchControl;
  std::shared_ptr<SearchControl> cancellable_search(engine::uci::SearchConfig config);

private:
  chess::Board current_board;
  KillerMoves killer_moves;
  TranspositionTable transposition_table;
  HistoryHeuristic history_heuristic;
  chess::StackRepetitionTracker repetition_tracker;
  std::mutex search_mutex;  // Only one search can run at anytime.

  struct SearchContext {
    DebugInfo debug;
    time_point cutoff_time;
    bool stopped;
    enum class TimeoutDanger { Low, Normal, High } timeout_danger;
    int root_depth;
    SearchControl* control;

    explicit SearchContext(time_point cutoff_time, SearchControl* control);

    // Returns true if the search should stop as soon as possible.
    bool should_stop();

  private:
    void update_timeout_danger(time_point current_time = std::chrono::steady_clock::now());
  };

  // Evaluate the current board state, without searching any further.
  int evaluate_board(const chess::Board& board);

  // Evaluate the priority of the given move. Higher priority moves should be searched first.
  int evaluate_move_priority(const chess::Move& move, int depth_left, const chess::Move& hash_move, bool is_white);

  // Evaluate the priority of the given quiescence move. Higher priority moves should be searched first.
  int evaluate_quiescence_move_priority(const chess::Move& move);

  // Continue traversing the search tree. Returns the evaluation and best move for the current player.
  // If `timed_out` is true, then the search aborted midway and the results are invalid.
  // If Move::null was returned as the best move, then it is not known what the best move is (e.g. due to null pruning).
  std::pair<int, chess::Move> search(const chess::Board& board, int alpha, int beta, int depth_left,
                                     SearchContext& context);

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player. Note that `depth_left` starts from 0 and decreases, so that all `depth_left`
  // in quiescence search is lower than in normal search.
  int quiescence_search(const chess::Board& board, int alpha, int beta, int depth_left, SearchContext& context);

  // Clears outdated information between each search depth in iterative deepening.
  void reset_iteration();

  // Clears outdated information between different board positions when choosing move.
  void reset_search();

  // Search at the given depth. Returns the best move if search completes before end_time, else returns Move::null().
  chess::Move iterative_deepening(int search_depth, SearchContext& context);
};

// A threadsafe interface for interacting with a cancellable search.
class Engine::SearchControl {
public:
  explicit SearchControl();

  // Check if the engine should stop searching.
  [[nodiscard]] bool is_stopped() const;

  // Tells the engine to stop searching as soon as possible.
  void stop();

  // Waits for the engine to finish searching and store the best move.
  void wait_for_done() const;

  // Sets the best move found, and signals that the search is done.
  //! TODO: This should be called exactly once by the Engine. It should also not be a public function available to
  //! external code. Calling it more than once will result in UB due to concurrent rw to `best_move`.
  void set_move(chess::Move move);

  // Moves the given thread into this search_thread.
  //! TODO: This should not be a public function available to external code.
  void run(std::thread thread);

  // Returns the best move found, or std::nullopt if the search is not finished yet.
  // This is guaranteed to return a value if `wait_for_done` has been called before this.
  [[nodiscard]] std::optional<chess::Move> try_get_move() const;

  // Waits until the search is done and returns the best move.
  [[nodiscard]] chess::Move wait_and_get_move() const;

  SearchControl(const SearchControl&) = delete;
  SearchControl(SearchControl&&) = delete;
  SearchControl& operator=(const SearchControl&) = delete;
  SearchControl& operator=(SearchControl&&) = delete;
  ~SearchControl();

private:
  std::atomic<bool> stopped;
  std::atomic<bool> done;
  std::thread search_thread;
  // The variable `done` starts as false, and will be set to true with memory_order::release after a write to
  // `best_move`. Hence, all reads to `best_move` should be preceeded by a true load of `done` with
  // memory_order::acquire, to ensure no concurrent access.
  chess::Move best_move;
};