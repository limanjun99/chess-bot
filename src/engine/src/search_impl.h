#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "chess/stack_repetition_tracker.h"
#include "evaluation.h"
#include "heuristics.h"
#include "search.h"
#include "time_management.h"
#include "uci.h"

class engine::Search::Impl {
public:
  explicit Impl(chess::Board position_, chess::StackRepetitionTracker repetition_tracker_,
                std::shared_ptr<Heuristics> heuristics_, engine::uci::SearchConfig config_);

  Impl(const Impl&) = delete;
  Impl(Impl&&) = delete;
  Impl& operator=(const Impl&) = delete;
  Impl& operator=(Impl&&) = delete;
  ~Impl();

  // Informs the search to stop as soon as possible.
  void stop();

  // Waits for the search to complete.
  void wait_for_done() const;

  // Waits for the search to complete, then returns the best move.
  chess::Move get_move() const;

  // Waits for the search to complete, then returns the debug information.
  DebugInfo get_debug_info() const;

  // Constructs an engine::Search from an instance of its underlying implementation.
  static std::shared_ptr<engine::Search> to_search(std::unique_ptr<Impl> impl);

private:
  // Concurrent RW to non-atomic private members is prevented by the `done` member.
  // When `done` is false, the search is ongoing in `search_thread` and will modify the members.
  // At the same time, reads through the public API are blocked.
  // When `done` is set to true, `search_thread` will no longer access the members, and
  // reads become permitted through the public API.

  chess::Board starting_position;
  chess::StackRepetitionTracker repetition_tracker;
  // This is the only variable not owned by Search::Impl, as it is too costly to copy it per search.
  std::shared_ptr<Heuristics> heuristics;
  engine::uci::SearchConfig config;
  std::atomic<bool> stop_signal;  // If true, the search has been signalled to stop.
  bool stopped;                   // If true, the engine has registered that search should stop.
  std::atomic<bool> done;         // If true, the search has completed.
  std::thread search_thread;
  chess::Move best_move;
  DebugInfo debug_info;
  int32_t root_depth;
  TimeManagement time_management;

  // Begin searching.
  void go(std::unique_lock<std::mutex> search_lock);

  // Continue traversing the search tree. Returns the evaluation and best move for the current player.
  // If `timed_out` is true, then the search aborted midway and the results are invalid.
  // If Move::null was returned as the best move, then it is not known what the best move is (e.g. due to null pruning).
  std::pair<Evaluation, chess::Move> search(const chess::Board& board, Evaluation alpha, Evaluation beta,
                                            int32_t depth_left);

  // Traverse the search tree until a position with no captures or max depth is reached. Returns the evaluation of the
  // current board for the current player. Note that `depth_left` starts from 0 and decreases, so that all `depth_left`
  // in quiescence search is lower than in normal search.
  Evaluation quiescence_search(const chess::Board& board, Evaluation alpha, Evaluation beta, int32_t depth_left);

  // Clears outdated information between each search depth in iterative deepening.
  void reset_iteration();

  // Search with depth of `root_depth`.
  // Returns the best move if search completes in time, else returns Move::null().
  chess::Move iterative_deepening();

  // Returns true if the search should stop as soon as possible.
  bool should_stop();
};