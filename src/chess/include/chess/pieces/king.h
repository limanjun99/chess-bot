#include "../bitboard.h"
#include "base_piece.h"

class King : public BasePiece<King> {
public:
  // Returns a bitboard of squares attacked by a king on `square`.
  static u64 attacks(u64 square);

private:
  static constexpr char character{'k'};
  static constexpr bool slider{false};
  static constexpr PieceVariant variant{PieceVariant::King};
  friend class BasePiece<King>;
};