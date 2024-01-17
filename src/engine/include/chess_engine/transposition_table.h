#pragma once

#include "chess/bitboard.h"
#include "chess/board.h"
#include "config.h"

enum class NodeType {
  PV,   // Nodes with score within [alpha, beta].
  Cut,  // Nodes with a score above beta. The stored score is a lowerbound.
  All   // Nodes with a score below alpha. The stored score is a upperbound.
};

struct PositionInfo {
  u64 hash;
  int depth_left;
  Move best_move;
  NodeType node_type;
  int score;

  PositionInfo();
  PositionInfo(u64 hash, int depth_left, Move best_move, NodeType node_type, int score);
};

class TranspositionTable {
public:
  TranspositionTable();

  // Returns a reference to the entry at the given hash.
  const PositionInfo& get(u64 hash) const;

  // Update the entry at the given hash.
  void update(u64 hash, int depth_left, Move best_move, NodeType node_type, int score);

  TranspositionTable(const TranspositionTable&) = delete;
  TranspositionTable(TranspositionTable&&) = delete;
  TranspositionTable& operator=(const TranspositionTable&) = delete;
  TranspositionTable& operator=(TranspositionTable&&) = delete;

  ~TranspositionTable();

private:
  PositionInfo* table;
};