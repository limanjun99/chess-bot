#include "transposition_table.h"

#include <array>
#include <random>

#include "config.h"

PositionInfo::PositionInfo() : hash{chess::Board::Hash::null} {}

PositionInfo::PositionInfo(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type,
                           int16_t score)
    : hash{hash},
      best_move{best_move},
      node_type{node_type},
      depth_left{static_cast<int8_t>(depth_left)},
      score{score} {}

chess::Move PositionInfo::get_best_move() const { return best_move; }

TranspositionTable::TranspositionTable() : table(config::transposition_table_size) {}

const PositionInfo& TranspositionTable::get(chess::Board::Hash hash) const {
  return table[hash.to_index(config::transposition_table_size)];
}

void TranspositionTable::update(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type,
                                int16_t score) {
  size_t index{hash.to_index(config::transposition_table_size)};
  if (table[index].hash == hash && table[index].depth_left > depth_left) return;
  table[index] = PositionInfo{hash, depth_left, best_move, node_type, score};
}
