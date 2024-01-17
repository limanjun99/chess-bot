#pragma once

#include "chess/bitboard.h"
#include "chess/board.h"

namespace board_hash {
// Returns the Zobrist hash of the given board.
u64 hash(const Board& board);
}  // namespace board_hash
