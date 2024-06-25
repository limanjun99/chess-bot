#include "stack_repetition_tracker.h"

#include "constants.h"

namespace chess {

void StackRepetitionTracker::push(const Board& board, const Move& move) {
  if (!move.is_null() && (move.is_capture() || move.is_castle() || move.get_piece() == PieceVariant::Pawn)) {
    // Irreversible move. Insert two null hashes to indicate that earlier positions need
    // not be considered for draw by repetition.
    positions.push_back(Position{Board::Hash::null, 1});
    positions.push_back(Position{Board::Hash::null, 0});
  }

  const Board::Hash hash{board.get_hash()};
  int repetition_count{1};
  for (int i{static_cast<int>(positions.size()) - 2}; i >= 0; i -= 2) {
    if (positions[i].hash.is_null()) break;
    if (positions[i].hash != hash) continue;
    repetition_count = positions[i].repetition_count + 1;
    break;
  }

  positions.push_back(Position{hash, repetition_count});
}

void StackRepetitionTracker::pop() {
  positions.pop_back();
  while (!positions.empty() && positions.back().hash.is_null()) positions.pop_back();
}

bool StackRepetitionTracker::is_repetition_draw() const {
  if (positions.empty()) return false;
  return positions.back().repetition_count >= threefold_repetition;
}

}  // namespace chess