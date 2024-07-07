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

std::pair<int, int> PositionInfo::get_score_bounds() const {
  switch (node_type) {
    case NodeType::PV:
      return std::pair{score, score};
    case NodeType::Cut:
      return std::pair{score, evaluation::max};
    case NodeType::All:
      return std::pair{evaluation::min, score};
  }
}

TranspositionTable::TranspositionTable() : table(config::transposition_table_size) {}

const PositionInfo* TranspositionTable::get(chess::Board::Hash hash) const {
  const size_t index{hash.to_index(config::transposition_table_size)};
  if (table[index].hash != hash) return nullptr;  // No matching entry found.
  return &table[index];
}

void TranspositionTable::try_update(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type,
                                    int16_t score) {
  const size_t index{hash.to_index(config::transposition_table_size)};
  if (table[index].depth_left > depth_left + 1) return;  // Existing entry is much superior.
  if (table[index].hash == hash) return;                 // Same entry already exists.
  table[index] = PositionInfo{hash, depth_left, best_move, node_type, score};
}
