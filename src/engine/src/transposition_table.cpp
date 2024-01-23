#include "transposition_table.h"

#include <array>
#include <random>

CompactMove::CompactMove() : from_index{0} {}

CompactMove::CompactMove(int8_t from_index, int8_t to_index, Piece piece, Piece captured_piece, Piece promotion_piece)
    : from_index{from_index},
      to_index{to_index},
      piece{piece},
      captured_piece{captured_piece},
      promotion_piece{promotion_piece} {}

CompactMove CompactMove::from_move(Move& move) {
  return CompactMove(bit::to_index(move.get_from()), bit::to_index(move.get_to()), move.get_piece(),
                     move.get_captured_piece(), move.get_promotion_piece());
}

Move CompactMove::to_move() const {
  if (promotion_piece != Piece::None) {
    return Move(piece, u64(1) << from_index, u64(1) << to_index, captured_piece);
  }
  return Move(u64(1) << from_index, u64(1) << to_index, promotion_piece, captured_piece);
}

PositionInfo::PositionInfo() : hash{0} {}

PositionInfo::PositionInfo(u64 hash, int depth_left, Move best_move, NodeType node_type, int score)
    : hash{hash},
      best_move{CompactMove::from_move(best_move)},
      node_type{node_type},
      depth_left{static_cast<int8_t>(depth_left)},
      score{score} {}

Move PositionInfo::get_best_move() const { return best_move.to_move(); }

TranspositionTable::TranspositionTable() : table{new PositionInfo[config::transposition_table_size]} {}

const PositionInfo& TranspositionTable::get(u64 hash) const { return table[hash % config::transposition_table_size]; }

void TranspositionTable::update(u64 hash, int depth_left, Move best_move, NodeType node_type, int score) {
  int index = hash % config::transposition_table_size;
  if (table[index].hash == hash && table[index].depth_left > depth_left) return;
  table[hash % config::transposition_table_size] = PositionInfo{hash, depth_left, best_move, node_type, score};
}

TranspositionTable::~TranspositionTable() { delete[] table; }
