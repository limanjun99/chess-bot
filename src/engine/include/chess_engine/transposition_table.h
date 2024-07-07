#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "chess/board.h"

enum class NodeType : int8_t {
  PV,   // Nodes with score within [alpha, beta].
  Cut,  // Nodes with a score above beta. The stored score is a lowerbound.
  All   // Nodes with a score below alpha. The stored score is a upperbound.
};

struct PositionInfo {
  chess::Board::Hash hash;
  chess::Move best_move;
  NodeType node_type;
  int8_t depth_left;
  int16_t score;

  PositionInfo();
  PositionInfo(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type, int16_t score);

  chess::Move get_best_move() const;

  std::pair<int, int> get_score_bounds() const;
};

class TranspositionTable {
public:
  TranspositionTable();

  // Returns a pointer to the entry at the given hash, or nullptr if no matching entry was found.
  const PositionInfo* get(chess::Board::Hash hash) const;

  // Try to update the entry at the given hash. Only succeeds if this new entry is analyzed to a greater depth than the
  // existing entry.
  void try_update(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type, int16_t score);

private:
  std::vector<PositionInfo> table;
};