#include "transposition_table.h"

#include <_types/_uint64_t.h>

#include <array>
#include <random>

PositionInfo::PositionInfo() : hash{0} {}

PositionInfo::PositionInfo(uint64_t hash, int depth_left, chess::Move best_move, NodeType node_type, int16_t score)
    : hash{hash},
      best_move{best_move},
      node_type{node_type},
      depth_left{static_cast<int8_t>(depth_left)},
      score{score} {}

chess::Move PositionInfo::get_best_move() const { return best_move; }

TranspositionTable::TranspositionTable() : table{new PositionInfo[config::transposition_table_size]} {}

const PositionInfo& TranspositionTable::get(uint64_t hash) const {
  return table[hash % config::transposition_table_size];
}

void TranspositionTable::update(uint64_t hash, int depth_left, chess::Move best_move, NodeType node_type,
                                int16_t score) {
  int index = hash % config::transposition_table_size;
  if (table[index].hash == hash && table[index].depth_left > depth_left) return;
  table[index] = PositionInfo{hash, depth_left, best_move, node_type, score};
}

TranspositionTable::~TranspositionTable() { delete[] table; }
