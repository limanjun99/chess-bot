#pragma once

#include "pieces/base_piece.h"
#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/queen.h"
#include "pieces/rook.h"

namespace piece_variant {
// Returns the corresponding PieceVariant given the piece's character (can be lowercase / uppercase).
constexpr PieceVariant from_char(char c);

// Returns true if the piece is a sliding piece.
constexpr bool is_slider(PieceVariant piece);

// Returns the corresponding lowercase character of the piece.
constexpr char to_char(PieceVariant piece);
}  // namespace piece_variant

// =============== IMPLEMENTATIONS ===============
//! TODO: Surely this is a terrible way of trying to get runtime polymorphism. Figure out how to fix this duplication.

constexpr PieceVariant piece_variant::from_char(char c) {
  // std::tolower is not constexpr.
  constexpr auto char_to_lower = [](char c) {
    if (c >= 'a' && c <= 'z') return c;
    return static_cast<char>(c + 32);
  };
  switch (char_to_lower(c)) {
    case Bishop::get_character():
      return Bishop::get_variant();
    case King::get_character():
      return King::get_variant();
    case Knight::get_character():
      return Knight::get_variant();
    case Pawn::get_character():
      return Pawn::get_variant();
    case Queen::get_character():
      return Queen::get_variant();
    case Rook::get_character():
      return Rook::get_variant();
    default:
      throw "Unreachable";
  }
}

constexpr bool piece_variant::is_slider(PieceVariant piece) {
  switch (piece) {
    case Bishop::get_variant():
      return Bishop::is_slider();
    case King::get_variant():
      return King::is_slider();
    case Knight::get_variant():
      return Knight::is_slider();
    case Pawn::get_variant():
      return Pawn::is_slider();
    case Queen::get_variant():
      return Queen::is_slider();
    case Rook::get_variant():
      return Rook::is_slider();
    default:
      throw "Unreachable";
  }
}

constexpr char piece_variant::to_char(PieceVariant piece) {
  switch (piece) {
    case Bishop::get_variant():
      return Bishop::get_character();
    case King::get_variant():
      return King::get_character();
    case Knight::get_variant():
      return Knight::get_character();
    case Pawn::get_variant():
      return Pawn::get_character();
    case Queen::get_variant():
      return Queen::get_character();
    case Rook::get_variant():
      return Rook::get_character();
    default:
      throw "Unreachable";
  }
}
