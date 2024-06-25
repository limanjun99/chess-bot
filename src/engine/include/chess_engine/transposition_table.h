#pragma once

#include <cstdint>
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
};

class TranspositionTable {
public:
  TranspositionTable();

  // Returns a reference to the entry at the given hash.
  const PositionInfo& get(chess::Board::Hash hash) const;

  // Update the entry at the given hash.
  void update(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type, int16_t score);

private:
  std::vector<PositionInfo> table;
};