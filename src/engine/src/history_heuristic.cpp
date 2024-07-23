#include "history_heuristic.h"

#include "chess/piece.h"

HistoryHeuristic::HistoryHeuristic() { clear(); }

void HistoryHeuristic::add_move_success(bool is_white, chess::Bitboard from, chess::Bitboard to) {
  const int from_index = from.to_index();
  const int to_index = to.to_index();
  successes[is_white][from_index][to_index]++;
  totals[is_white][from_index][to_index]++;
}

void HistoryHeuristic::add_move_failure(bool is_white, chess::Bitboard from, chess::Bitboard to) {
  const int from_index = from.to_index();
  const int to_index = to.to_index();
  totals[is_white][from_index][to_index]++;
}

void HistoryHeuristic::clear() {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 64; j++) {
      for (int k = 0; k < 64; k++) {
        successes[i][j][k] = 0;
        totals[i][j][k] = 0;
      }
    }
  }
}

double HistoryHeuristic::get_ratio(chess::Color player_color, chess::Bitboard from, chess::Bitboard to) {
  const int from_index = from.to_index();
  const int to_index = to.to_index();
  return static_cast<double>(successes[player_color.to_index()][from_index][to_index]) /
         (totals[player_color.to_index()][from_index][to_index] + 1);
}
