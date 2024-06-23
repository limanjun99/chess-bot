#include "killer_moves.h"

#include <utility>

void KillerMoves::add(const chess::Move& move, int depth_left) {
  for (int i = 0; i < config::killer_move_count; i++) {
    if (killer_moves[depth_left][i] == move) {
      std::swap(killer_moves[depth_left][i], killer_moves[depth_left][0]);
      return;
    }
  }

  for (int i = config::killer_move_count - 1; i > 0; i--) {
    killer_moves[depth_left][i] = killer_moves[depth_left][i - 1];
  }
  killer_moves[depth_left][0] = move;
}

void KillerMoves::clear() {
  for (int i = 0; i < config::max_depth; i++) {
    for (int j = 0; j < config::killer_move_count; j++) {
      killer_moves[i][j] = chess::Move{};
    }
  }
}

const chess::Move& KillerMoves::get(int depth_left, int index) const { return killer_moves[depth_left][index]; }
