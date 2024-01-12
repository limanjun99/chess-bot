#pragma once

#include "chess/board.h"
#include "chess/move.h"

class Engine {
public:
  // Contains information about the move chosen by the engine.
  struct MoveInfo {
    Move move;
    int time_ms;                // Time in milliseconds spent searching.
    int normal_node_count;      // Number of nodes in the main search tree.
    int quiescence_node_count;  // Number of nodes in quiescence search.
  };
  // Choose a move to make for the current player of the given board.
  virtual MoveInfo make_move(const Board& board) = 0;

  virtual ~Engine() = default;
};