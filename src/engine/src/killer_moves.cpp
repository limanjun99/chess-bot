#include "killer_moves.h"

#include <algorithm>

void KillerMoves::add(const chess::Move& move, int depth_left) {
  auto& moves = killer_moves[depth_left];
  const auto it = std::ranges::find(moves, move);
  if (it == moves.end()) {
    std::move_backward(moves.begin(), moves.end() - 1, moves.end());
  } else {
    std::move_backward(moves.begin(), it, it + 1);
  }
  moves[0] = move;
}

void KillerMoves::clear() {
  for (auto& array : killer_moves) {
    std::ranges::fill(array, chess::Move::null());
  }
}

const chess::Move& KillerMoves::get(int depth_left, int index) const { return killer_moves[depth_left][index]; }
