#pragma once

#include <mutex>

#include "history_heuristic.h"
#include "killer_moves.h"
#include "transposition_table.h"

// A struct to aggregate all the data used for various heuristics during search.
struct Heuristics {
  KillerMoves killer_moves;
  TranspositionTable transposition_table;
  HistoryHeuristic history_heuristic;
  std::mutex mutex;  // Any access to the data should lock this mutex first.
};
