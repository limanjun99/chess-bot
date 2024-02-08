#include "../bitboard.h"
#include "base_piece.h"

class Pawn : public BasePiece<Pawn> {
public:
  // Returns a bitboard of squares attacked by a `PieceColor` pawn on `square`.
  template <Color PieceColor>
  static u64 attacks(u64 square);

  // Returns a bitboard of squares that a `PieceColor` pawn on `square` can push to, given that there are pieces on
  // `occupancy`.
  template <Color PieceColor>
  static u64 pushes(u64 square, u64 occupancy);

  // Returns the promotion squares of a piece of the given `PieceColor`.
  template <Color PieceColor>
  static constexpr u64 get_promotion_squares();

private:
  static constexpr char character{'p'};
  static constexpr bool slider{false};
  static constexpr PieceVariant variant{PieceVariant::Pawn};
  friend class BasePiece<Pawn>;
};

// =============== IMPLEMENTATIONS ===============

template <>
inline u64 Pawn::attacks<Color::Black>(u64 square) {
  const u64 left = square >> 9;
  const u64 right = square >> 7;
  return (left & ~bitboard::FILE_H) | (right & ~bitboard::FILE_A);
}

template <>
inline u64 Pawn::attacks<Color::White>(u64 square) {
  const u64 left = square << 7;
  const u64 right = square << 9;
  return (left & ~bitboard::FILE_H) | (right & ~bitboard::FILE_A);
}

template <>
inline u64 Pawn::pushes<Color::Black>(u64 square, u64 occupancy) {
  return ((((square & bitboard::RANK_7) >> 8 & ~occupancy) | square) >> 8) & ~occupancy;
}

template <>
inline u64 Pawn::pushes<Color::White>(u64 square, u64 occupancy) {
  return ((((square & bitboard::RANK_2) << 8 & ~occupancy) | square) << 8) & ~occupancy;
}

template <Color PieceColor>
inline constexpr u64 Pawn::get_promotion_squares() {
  if constexpr (PieceColor == Color::White) return bitboard::RANK_8;
  else return bitboard::RANK_1;
}