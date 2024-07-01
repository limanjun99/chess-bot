#pragma once

#include <type_traits>

#include "pieces/base_piece.h"
#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/queen.h"
#include "pieces/rook.h"

namespace chess {

// clang-format off
template <PieceType Type>
using Piece =
  std::conditional_t<Type == PieceType::Bishop, Bishop,
  std::conditional_t<Type == PieceType::King, King,
  std::conditional_t<Type == PieceType::Knight, Knight,
  std::conditional_t<Type == PieceType::Pawn, Pawn,
  std::conditional_t<Type == PieceType::Queen, Queen,
  std::conditional_t<Type == PieceType::Rook, Rook,
  void>>>>>>;
// clang-format on

namespace piece {

// Calls the function Func with the correct PieceType of the given piece.
template <typename Func>
constexpr auto visit(PieceType piece, const Func& func);

// Calls the function Func with all pieces.
template <typename Func>
constexpr auto visit_all_pieces(const Func& func);

// Calls the function Func with all pieces (excluding the king).
template <typename Func>
constexpr auto visit_non_king_pieces(const Func& func);

// Returns the corresponding PieceType given the piece's character (can be lowercase / uppercase).
constexpr PieceType from_char(char c);

// Returns true if the piece is a sliding piece.
constexpr bool is_slider(PieceType piece);

// Returns the corresponding lowercase character of the piece.
constexpr char to_char(PieceType piece);

// Returns the corresponding lowercase / uppercase character of the black / white piece.
template <Color PieceColor>
constexpr char to_colored_char(PieceType piece);

}  // namespace piece

// =============== IMPLEMENTATIONS ===============

template <typename Func>
constexpr auto piece::visit(PieceType piece, const Func& func) {
  switch (piece) {
    case PieceType::Bishop:
      return func.template operator()<PieceType::Bishop>();
    case PieceType::King:
      return func.template operator()<PieceType::King>();
    case PieceType::Knight:
      return func.template operator()<PieceType::Knight>();
    case PieceType::Pawn:
      return func.template operator()<PieceType::Pawn>();
    case PieceType::Queen:
      return func.template operator()<PieceType::Queen>();
    case PieceType::Rook:
      return func.template operator()<PieceType::Rook>();
    default:
      throw "Invalid piece type.";
  }
}

template <typename Func>
constexpr auto piece::visit_all_pieces(const Func& func) {
  func.template operator()<PieceType::Bishop>();
  func.template operator()<PieceType::King>();
  func.template operator()<PieceType::Knight>();
  func.template operator()<PieceType::Pawn>();
  func.template operator()<PieceType::Queen>();
  func.template operator()<PieceType::Rook>();
}

template <typename Func>
constexpr auto piece::visit_non_king_pieces(const Func& func) {
  func.template operator()<PieceType::Bishop>();
  func.template operator()<PieceType::Knight>();
  func.template operator()<PieceType::Pawn>();
  func.template operator()<PieceType::Queen>();
  func.template operator()<PieceType::Rook>();
}

constexpr PieceType piece::from_char(char c) {
  // std::tolower is not constexpr.
  if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  switch (c) {
    case Bishop::get_character():
      return Bishop::get_type();
    case King::get_character():
      return King::get_type();
    case Knight::get_character():
      return Knight::get_type();
    case Pawn::get_character():
      return Pawn::get_type();
    case Queen::get_character():
      return Queen::get_type();
    case Rook::get_character():
      return Rook::get_type();
    default:
      throw "Unreachable";
  }
}

constexpr bool piece::is_slider(PieceType piece) {
  constexpr auto piece_is_slider{[]<PieceType Type>() { return Piece<Type>::is_slider(); }};
  return visit(piece, piece_is_slider);
}

constexpr char piece::to_char(PieceType piece) {
  constexpr auto piece_to_char{[]<PieceType Type>() { return Piece<Type>::get_character(); }};
  return visit(piece, piece_to_char);
}

template <Color PieceColor>
constexpr char piece::to_colored_char(PieceType piece) {
  constexpr auto piece_to_colored_char{[]<PieceType Type>() {
    char c{Piece<Type>::get_character()};
    if constexpr (PieceColor == Color::White) return c + ('A' - 'a');
    else return c;
  }};
  return visit(piece, piece_to_colored_char);
}

}  // namespace chess