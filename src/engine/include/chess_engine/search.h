#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include "chess/move.h"

namespace engine {

class Search {
public:
  struct DebugInfo {
    int16_t evaluation;                    // Evaluation of the board position (in centipawns).
    int64_t normal_node_count;             // Number of nodes in the main search tree.
    int64_t quiescence_node_count;         // Number of nodes in quiescence search.
    int64_t null_move_success;             // Nodes that returned after a null move search.
    int64_t null_move_total;               // Nodes that did a null move search.
    int64_t transposition_table_success;   // Nodes that returned immediately after checking transposition table.
    int64_t transposition_table_total;     // Nodes that checked the transposition table.
    int64_t q_delta_pruning_success;       // Nodes that were delta pruned.
    int64_t q_delta_pruning_total;         // Nodes that tried to delta prune.
    int32_t search_depth;                  // Maximum depth reached during search.
    std::chrono::milliseconds time_spent;  // Time in milliseconds spent searching.
  };

  //! TODO: This really should be a private class, but I can't
  //! figure out how to make the constructor accessible, even when friending.
  class Impl;
  explicit Search(std::unique_ptr<Impl> impl);

  // Informs the search to stop as soon as possible.
  void stop();

  // Waits for the search to complete.
  void wait_for_done() const;

  // Waits for the search to complete, then returns the best move.
  [[nodiscard]] chess::Move get_move() const;

  // Waits for the search to complete, then returns the debug information.
  [[nodiscard]] DebugInfo get_debug_info() const;

  Search(const Search&) = delete;
  Search(Search&&) = default;
  Search& operator=(const Search&) = delete;
  Search& operator=(Search&&) = default;
  ~Search();

private:
  std::unique_ptr<Impl> impl;
};

}  // namespace engine