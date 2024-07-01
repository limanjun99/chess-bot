#pragma once

#include <cctype>
#include <cstdint>

#include "../color.h"

namespace chess {

enum class PieceType : uint8_t { Queen = 0, Rook = 1, Bishop = 2, Knight = 3, Pawn = 4, King = 5, None = 6 };

template <typename Derived>
class BasePiece {
public:
  // Returns the lowercase character representation of the piece.
  static constexpr char get_character();

  // Returns the lowercase / uppercase (black / white) character representation of the piece.
  template <Color PieceColor>
  static constexpr char get_colored_char();

  // Returns the corresponding PieceType of the piece.
  static constexpr PieceType get_type();

  // Returns true if the piece is a sliding piece.
  static constexpr bool is_slider();
};

// =============== IMPLEMENTATIONS ===============

template <typename Derived>
inline constexpr char BasePiece<Derived>::get_character() {
  return Derived::character;
}

template <typename Derived>
template <Color PieceColor>
inline constexpr char BasePiece<Derived>::get_colored_char() {
  if constexpr (PieceColor == Color::Black) {
    return Derived::character;
  } else {
    return toupper(Derived::character);
  }
}

template <typename Derived>
inline constexpr PieceType BasePiece<Derived>::get_type() {
  return Derived::type;
}

template <typename Derived>
inline constexpr bool BasePiece<Derived>::is_slider() {
  return Derived::slider;
}

}  // namespace chess