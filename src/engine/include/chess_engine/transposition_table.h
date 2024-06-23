#pragma once

#include <stdint.h>

#include "chess/bitboard.h"
#include "chess/board.h"
#include "config.h"

enum class NodeType : int8_t {
  PV,   // Nodes with score within [alpha, beta].
  Cut,  // Nodes with a score above beta. The stored score is a lowerbound.
  All   // Nodes with a score below alpha. The stored score is a upperbound.
};

struct PositionInfo {
  uint64_t hash;
  chess::Move best_move;
  NodeType node_type;
  int8_t depth_left;
  int16_t score;

  PositionInfo();
  PositionInfo(uint64_t hash, int depth_left, chess::Move best_move, NodeType node_type, int16_t score);

  // Reconstruct a Move object from the stored CompactMove object.
  chess::Move get_best_move() const;
};

class TranspositionTable {
public:
  TranspositionTable();

  // Returns a reference to the entry at the given hash.
  const PositionInfo& get(uint64_t hash) const;

  // Update the entry at the given hash.
  void update(uint64_t hash, int depth_left, chess::Move best_move, NodeType node_type, int16_t score);

  TranspositionTable(const TranspositionTable&) = delete;
  TranspositionTable(TranspositionTable&&) = delete;
  TranspositionTable& operator=(const TranspositionTable&) = delete;
  TranspositionTable& operator=(TranspositionTable&&) = delete;

  ~TranspositionTable();

private:
  PositionInfo* table;
};