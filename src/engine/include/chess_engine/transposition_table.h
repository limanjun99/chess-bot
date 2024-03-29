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

// Used in the transposition table to save space. A compact move has the following format:
// aaaaaabbbbbbcccdddeee
// aaaaaa = 6 bit `from_index`.
// bbbbbb = 6 bit `to_index`.
// ccc = 3 bit `piece`.
// ddd = 3 bit `captured_piece`.
// eee = 3 bit `promotion_piece`.
class CompactMove {
public:
  CompactMove();
  static CompactMove from_move(Move& move);
  Move to_move() const;

private:
  int32_t value;

  CompactMove(int from_index, int to_index, PieceVariant piece, PieceVariant captured_piece,
              PieceVariant promotion_piece);
  int get_from_index() const;
  int get_to_index() const;
  PieceVariant get_piece() const;
  PieceVariant get_captured_piece() const;
  PieceVariant get_promotion_piece() const;
};

struct PositionInfo {
  u64 hash;
  CompactMove best_move;
  NodeType node_type;
  int8_t depth_left;
  int16_t score;

  PositionInfo();
  PositionInfo(u64 hash, int depth_left, Move best_move, NodeType node_type, int16_t score);

  // Reconstruct a Move object from the stored CompactMove object.
  Move get_best_move() const;
};

class TranspositionTable {
public:
  TranspositionTable();

  // Returns a reference to the entry at the given hash.
  const PositionInfo& get(u64 hash) const;

  // Update the entry at the given hash.
  void update(u64 hash, int depth_left, Move best_move, NodeType node_type, int16_t score);

  TranspositionTable(const TranspositionTable&) = delete;
  TranspositionTable(TranspositionTable&&) = delete;
  TranspositionTable& operator=(const TranspositionTable&) = delete;
  TranspositionTable& operator=(TranspositionTable&&) = delete;

  ~TranspositionTable();

private:
  PositionInfo* table;
};