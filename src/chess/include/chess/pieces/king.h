#include "../bitboard.h"
#include "base_piece.h"

namespace chess {

class King : public BasePiece<King> {
public:
  // Returns a bitboard of squares attacked by a king on `square`.
  static Bitboard attacks(Bitboard square);

private:
  static constexpr char character{'k'};
  static constexpr bool slider{false};
  static constexpr PieceType type{PieceType::King};
  friend class BasePiece<King>;
};

}  // namespace chess