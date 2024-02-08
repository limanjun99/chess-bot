#include "transposition_table.h"

#include <array>
#include <random>

CompactMove::CompactMove() : value{0} {}

CompactMove::CompactMove(int from_index, int to_index, PieceVariant piece, PieceVariant captured_piece,
                         PieceVariant promotion_piece)
    : value{(from_index << 15) | (to_index << 9) | (static_cast<int>(piece) << 6) |
            (static_cast<int>(captured_piece) << 3) | (static_cast<int>(promotion_piece))} {}

int CompactMove::get_from_index() const { return value >> 15; }

int CompactMove::get_to_index() const { return (value >> 9) & 0b111111; }

PieceVariant CompactMove::get_piece() const { return static_cast<PieceVariant>((value >> 6) & 0b111); }

PieceVariant CompactMove::get_captured_piece() const { return static_cast<PieceVariant>((value >> 3) & 0b111); }

PieceVariant CompactMove::get_promotion_piece() const { return static_cast<PieceVariant>(value & 0b111); }

CompactMove CompactMove::from_move(Move& move) {
  return CompactMove(bit::to_index(move.get_from()), bit::to_index(move.get_to()), move.get_piece(),
                     move.get_captured_piece(), move.get_promotion_piece());
}

Move CompactMove::to_move() const {
  if ((value & 0b111) != static_cast<int>(PieceVariant::None)) {
    return Move(get_piece(), u64(1) << get_from_index(), u64(1) << get_to_index(), get_captured_piece());
  }
  return Move(u64(1) << get_from_index(), u64(1) << get_to_index(), get_promotion_piece(), get_captured_piece());
}

PositionInfo::PositionInfo() : hash{0} {}

PositionInfo::PositionInfo(u64 hash, int depth_left, Move best_move, NodeType node_type, int16_t score)
    : hash{hash},
      best_move{CompactMove::from_move(best_move)},
      node_type{node_type},
      depth_left{static_cast<int8_t>(depth_left)},
      score{score} {}

Move PositionInfo::get_best_move() const { return best_move.to_move(); }

TranspositionTable::TranspositionTable() : table{new PositionInfo[config::transposition_table_size]} {}

const PositionInfo& TranspositionTable::get(u64 hash) const { return table[hash % config::transposition_table_size]; }

void TranspositionTable::update(u64 hash, int depth_left, Move best_move, NodeType node_type, int16_t score) {
  int index = hash % config::transposition_table_size;
  if (table[index].hash == hash && table[index].depth_left > depth_left) return;
  table[index] = PositionInfo{hash, depth_left, best_move, node_type, score};
}

TranspositionTable::~TranspositionTable() { delete[] table; }
