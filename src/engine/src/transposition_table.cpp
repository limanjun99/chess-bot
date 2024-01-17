#include "transposition_table.h"

#include <array>
#include <random>

PositionInfo::PositionInfo() : hash{0} {}

PositionInfo::PositionInfo(u64 hash, int depth_left, Move best_move, NodeType node_type, int score)
    : hash{hash}, depth_left{depth_left}, best_move{best_move}, node_type{node_type}, score{score} {}

TranspositionTable::TranspositionTable() : table{new PositionInfo[config::transposition_table_size]} {}

const PositionInfo& TranspositionTable::get(u64 hash) const { return table[hash % config::transposition_table_size]; }

void TranspositionTable::update(u64 hash, int depth_left, Move best_move, NodeType node_type, int score) {
  int index = hash % config::transposition_table_size;
  if (table[index].hash == hash && table[index].depth_left > depth_left) return;
  table[hash % config::transposition_table_size] = PositionInfo{hash, depth_left, best_move, node_type, score};
}

TranspositionTable::~TranspositionTable() { delete[] table; }
