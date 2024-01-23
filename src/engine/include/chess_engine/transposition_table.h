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

class CompactMove {
public:
  CompactMove();
  static CompactMove from_move(Move& move);
  Move to_move() const;

private:
  int8_t from_index;
  int8_t to_index;
  Piece piece;
  Piece captured_piece;
  Piece promotion_piece;

  CompactMove(int8_t from_index, int8_t to_index, Piece piece, Piece captured_piece, Piece promotion_piece);
};

struct PositionInfo {
  u64 hash;
  CompactMove best_move;
  NodeType node_type;
  int8_t depth_left;
  int score;

  PositionInfo();
  PositionInfo(u64 hash, int depth_left, Move best_move, NodeType node_type, int score);

  // Reconstruct a Move object from the stored CompactMove object.
  Move get_best_move() const;
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