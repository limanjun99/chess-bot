#include "../bitboard.h"
#include "base_piece.h"

namespace chess {

class Pawn : public BasePiece<Pawn> {
public:
  // Returns a bitboard of squares attacked by a `PieceColor` pawn on `square`.
  template <Color PieceColor>
  static Bitboard attacks(Bitboard square);

  // Returns a bitboard of squares that a `PieceColor` pawn on `square` can push to, given that there are pieces on
  // `occupancy`.
  template <Color PieceColor>
  static Bitboard pushes(Bitboard square, Bitboard occupancy);

  // Returns the promotion squares of a piece of the given `PieceColor`.
  template <Color PieceColor>
  static constexpr Bitboard get_promotion_squares();

private:
  static constexpr char character{'p'};
  static constexpr bool slider{false};
  static constexpr PieceVariant variant{PieceVariant::Pawn};
  friend class BasePiece<Pawn>;
};

// =============== IMPLEMENTATIONS ===============

template <>
inline Bitboard Pawn::attacks<Color::Black>(Bitboard square) {
  const Bitboard left = square >> 9;
  const Bitboard right = square >> 7;
  return (left & ~bitboard::FILE_H) | (right & ~bitboard::FILE_A);
}

template <>
inline Bitboard Pawn::attacks<Color::White>(Bitboard square) {
  const Bitboard left = square << 7;
  const Bitboard right = square << 9;
  return (left & ~bitboard::FILE_H) | (right & ~bitboard::FILE_A);
}

template <>
inline Bitboard Pawn::pushes<Color::Black>(Bitboard square, Bitboard occupancy) {
  return ((((square & bitboard::RANK_7) >> 8 & ~occupancy) | square) >> 8) & ~occupancy;
}

template <>
inline Bitboard Pawn::pushes<Color::White>(Bitboard square, Bitboard occupancy) {
  return ((((square & bitboard::RANK_2) << 8 & ~occupancy) | square) << 8) & ~occupancy;
}

template <Color PieceColor>
inline constexpr Bitboard Pawn::get_promotion_squares() {
  if constexpr (PieceColor == Color::White) return bitboard::RANK_8;
  else return bitboard::RANK_1;
}

}  // namespace chess