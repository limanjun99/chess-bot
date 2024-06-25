#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "bitboard.h"
#include "chess/pieces/base_piece.h"

namespace chess {

// Move information is stored within 24 bits (or three uint8_t), with the following format:
// 1st uint8_t: index of the moved from square.
// 2nd uint8_t: index of the moved to square.
// 3rd uint8_t bits 0~1: the promotion piece (only relevant if moved piece is pawn, to first / last rank).
// 3rd uint8_t bits 2~4: the captured piece.
// 3rd uint8_t bits 5~7: the moved piece.
class Move {
public:
  // Construct a null move.
  constexpr explicit Move();

  // Construct a null move.
  static constexpr Move null();

  // Construct a move that is not a promotion.
  static constexpr Move move(Bitboard from, Bitboard to, PieceVariant from_piece);
  static constexpr Move move(Bitboard from, Bitboard to, PieceVariant from_piece, PieceVariant captured_piece);

  // Construct a promotion.
  static constexpr Move promotion(Bitboard from, Bitboard to, PieceVariant promotion_piece);
  static constexpr Move promotion(Bitboard from, Bitboard to, PieceVariant promotion_piece,
                                  PieceVariant captured_piece);

  // Getter for from bitboard.
  constexpr Bitboard get_from() const;

  // Getter for to bitboard.
  constexpr Bitboard get_to() const;

  // Getter for moved piece.
  constexpr PieceVariant get_piece() const;

  // Getter for captured piece.
  constexpr PieceVariant get_captured_piece() const;

  // Getter for promotion piece. Is UB if this is not a promotion.
  constexpr PieceVariant get_promotion_piece() const;

  // Whether this move is a capture.
  constexpr bool is_capture() const;

  // Whether this move is a castle.
  constexpr bool is_castle() const;

  // Whether this move is a promotion.
  constexpr bool is_promotion() const;

  // Whether this is a null move.
  constexpr bool is_null() const;

  // Returns this move in UCI format.
  std::string to_uci() const;

  // Returns this move in algebraic notation (e.g. Nf3).
  std::string to_algebraic() const;

  // Check equality between two moves.
  constexpr bool operator==(const Move& other) const;

private:
  uint8_t from;
  uint8_t to;
  uint8_t flag;

  // Note that PieceVariant::None is not required to fit within two bits.
  // Hence if we only have one constructor that must take a `promotion_piece` as argument,
  // and we want to construct a non-promotion move, we cannot pass in PieceVariant::None, which is kind of silly.
  // Hence the need for both constructors below.
  constexpr explicit Move(Bitboard from, Bitboard to, PieceVariant moved_piece, PieceVariant captured_piece);
  constexpr explicit Move(Bitboard from, Bitboard to, PieceVariant moved_piece, PieceVariant captured_piece,
                          PieceVariant promotion_piece);
};

// ========== IMPLEMENTATIONS ==========

// Assert that promotion pieces are < 4 so that it fits within 2 bits.
static_assert(static_cast<uint8_t>(PieceVariant::Bishop) < 4);
static_assert(static_cast<uint8_t>(PieceVariant::Knight) < 4);
static_assert(static_cast<uint8_t>(PieceVariant::Queen) < 4);
static_assert(static_cast<uint8_t>(PieceVariant::Rook) < 4);

constexpr Move::Move() : from{0}, to{0}, flag{0} {}

constexpr Move::Move(Bitboard from, Bitboard to, PieceVariant moved_piece, PieceVariant captured_piece)
    : from{static_cast<uint8_t>(from.to_index())},
      to{static_cast<uint8_t>(to.to_index())},
      flag{static_cast<uint8_t>(static_cast<uint8_t>(captured_piece) << 3 | static_cast<uint8_t>(moved_piece))} {}

constexpr Move::Move(Bitboard from, Bitboard to, PieceVariant moved_piece, PieceVariant captured_piece,
                     PieceVariant promotion_piece)
    : from{static_cast<uint8_t>(from.to_index())},
      to{static_cast<uint8_t>(to.to_index())},
      flag{static_cast<uint8_t>(static_cast<uint8_t>(promotion_piece) << 6 | static_cast<uint8_t>(captured_piece) << 3 |
                                static_cast<uint8_t>(moved_piece))} {}

constexpr Move Move::null() { return Move{}; }

constexpr Move Move::move(Bitboard from, Bitboard to, PieceVariant from_piece) {
  return Move{from, to, from_piece, PieceVariant::None};
}

constexpr Move Move::move(Bitboard from, Bitboard to, PieceVariant from_piece, PieceVariant captured_piece) {
  return Move{from, to, from_piece, captured_piece};
}

constexpr Move Move::promotion(Bitboard from, Bitboard to, PieceVariant promotion_piece) {
  return Move{from, to, PieceVariant::Pawn, PieceVariant::None, promotion_piece};
}

constexpr Move Move::promotion(Bitboard from, Bitboard to, PieceVariant promotion_piece, PieceVariant captured_piece) {
  return Move{from, to, PieceVariant::Pawn, captured_piece, promotion_piece};
}

constexpr Bitboard Move::get_from() const { return Bitboard::from_index(from); }

constexpr Bitboard Move::get_to() const { return Bitboard::from_index(to); }

constexpr PieceVariant Move::get_piece() const { return static_cast<PieceVariant>(flag & 0b111); }

constexpr PieceVariant Move::get_captured_piece() const { return static_cast<PieceVariant>((flag >> 3) & 0b111); }

constexpr PieceVariant Move::get_promotion_piece() const { return static_cast<PieceVariant>(flag >> 6); }

constexpr bool Move::is_capture() const { return get_captured_piece() != PieceVariant::None; }

constexpr bool Move::is_castle() const {
  if (get_piece() != PieceVariant::King) return false;
  const Bitboard from{get_from()};
  const Bitboard to{get_to()};
  return static_cast<bool>(to & (from << 2 | from >> 2));
}

constexpr bool Move::is_promotion() const {
  return get_piece() == PieceVariant::Pawn && (Bitboard::from_index(to) & (bitboard::RANK_1 | bitboard::RANK_8));
}

constexpr bool Move::is_null() const { return from == 0 && to == 0 && flag == 0; }

constexpr bool Move::operator==(const Move& other) const {
  return from == other.from && to == other.to && flag == other.flag;
}

}  // namespace chess