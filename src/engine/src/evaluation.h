#pragma once

#include "chess/board.h"

// Piece Square Tables
namespace pst {
// Returns the given board's evaluation based on PST.
int evaluate(const chess::Board& board);
};  // namespace pst