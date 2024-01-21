#include "history_heuristic.h"

#include "config.h"

HistoryHeuristic::HistoryHeuristic() { clear(); }

void HistoryHeuristic::add_move_success(bool is_white, u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
  successes[is_white][from_index][to_index]++;
  totals[is_white][from_index][to_index]++;
}

void HistoryHeuristic::add_move_failure(bool is_white, u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
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

int HistoryHeuristic::get_score(bool is_white, u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
  return successes[is_white][from_index][to_index] * move_priority::history_heuristic_scale /
         totals[is_white][from_index][to_index];
}
