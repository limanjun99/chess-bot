#include "transposition_table.h"

#include <utility>

#include "config.h"
#include "evaluation.h"

PositionInfo::PositionInfo() : hash{chess::Board::Hash::null} {}

PositionInfo::PositionInfo(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type,
                           Evaluation score)
    : hash{hash},
      best_move{best_move},
      node_type{node_type},
      depth_left{static_cast<int8_t>(depth_left)},
      score{score} {}

chess::Move PositionInfo::get_best_move() const { return best_move; }

std::pair<Evaluation, Evaluation> PositionInfo::get_score_bounds() const {
  switch (node_type) {
    case NodeType::PV:
      return std::make_pair(score, score);
    case NodeType::Cut:
      return std::make_pair(score, Evaluation::max);
    case NodeType::All:
      return std::make_pair(Evaluation::min, score);
    default:
      std::unreachable();
  }
}

TranspositionTable::TranspositionTable() : table(config::transposition_table_size) {}

const PositionInfo* TranspositionTable::get(chess::Board::Hash hash) const {
  const size_t index{hash.to_index(config::transposition_table_size)};
  if (table[index].hash != hash) return nullptr;  // No matching entry found.
  return &table[index];
}

void TranspositionTable::try_update(chess::Board::Hash hash, int depth_left, chess::Move best_move, NodeType node_type,
                                    Evaluation score) {
  const size_t index{hash.to_index(config::transposition_table_size)};
  if (table[index].depth_left > depth_left + 1) return;                            // Existing entry is much superior.
  if (table[index].hash == hash && table[index].depth_left >= depth_left) return;  // Same entry already exists.
  table[index] = PositionInfo{hash, depth_left, best_move, node_type, score};
}
