#pragma once

#include <optional>

#include "bitboard.h"
#include "piece.h"

class Move {
public:
  // Construct a move of the given piece.
  Move(Piece piece, u64 from, u64 to);

  // Construct a promotion to the given promotion_piece.
  Move(u64 from, u64 to, Piece promotion_piece);

private:
  Piece piece;
  u64 from;
  u64 to;
  std::optional<Piece> promotion_piece;
};