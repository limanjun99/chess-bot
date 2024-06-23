#pragma once

#include <cctype>
#include <cstdint>

#include "../color.h"

namespace chess {

enum class PieceVariant : int8_t { Bishop = 0, King = 1, Knight = 2, Pawn = 3, Queen = 4, Rook = 5, None = 6 };

template <typename Derived>
class BasePiece {
public:
  // Returns the lowercase character representation of the piece.
  static constexpr char get_character();

  // Returns the lowercase / uppercase (black / white) character representation of the piece.
  template <Color PieceColor>
  static constexpr char get_colored_char();

  // Returns the corresponding PieceVariant of the piece.
  static constexpr PieceVariant get_variant();

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
inline constexpr PieceVariant BasePiece<Derived>::get_variant() {
  return Derived::variant;
}

template <typename Derived>
inline constexpr bool BasePiece<Derived>::is_slider() {
  return Derived::slider;
}

}  // namespace chess