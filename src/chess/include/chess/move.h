#pragma once

#include <optional>
#include <string>

#include "bitboard.h"
#include "piece.h"

class Move {
public:
  // Construct a null move.
  Move();

  // Construct a move of the given piece.
  Move(Piece piece, u64 from, u64 to);

  // Construct a capture that is not a promotion.
  Move(Piece piece, u64 from, u64 to, Piece promoted_piece);

  // Construct a promotion to the given promotion_piece.
  Move(u64 from, u64 to, Piece promotion_piece);

  // Construct a promotion to the given promotion_piece that is also a capture.
  Move(u64 from, u64 to, Piece promotion_piece, Piece captured_piece);

  // Getter for from bitboard.
  u64 get_from() const;

  // Getter for to bitboard.
  u64 get_to() const;

  // Getter for moved piece.
  Piece get_piece() const;

  // Getter for captured piece.
  Piece get_captured_piece() const;

  // Getter for promotion piece.
  Piece get_promotion_piece() const;

  // Whether this move is a capture.
  bool is_capture() const;

  // Whether this move is a promotion.
  bool is_promotion() const;

  // Returns this move in UCI format.
  std::string to_uci() const;

  // Check equality between two moves.
  bool operator==(const Move& move) const;

private:
  u64 from;
  u64 to;
  Piece piece;
  Piece captured_piece;
  Piece promotion_piece;
};

inline Move::Move() : from{0}, to{0}, piece{Piece::None}, promotion_piece{Piece::None} {}

inline Move::Move(Piece piece, u64 from, u64 to)
    : from{from}, to{to}, piece{piece}, captured_piece{Piece::None}, promotion_piece{Piece::None} {}

inline Move::Move(Piece piece, u64 from, u64 to, Piece captured_piece)
    : from{from}, to{to}, piece{piece}, captured_piece{captured_piece}, promotion_piece{Piece::None} {}

inline Move::Move(u64 from, u64 to, Piece promotion_piece)
    : from{from}, to{to}, piece{Piece::Pawn}, captured_piece{Piece::None}, promotion_piece{promotion_piece} {}

inline Move::Move(u64 from, u64 to, Piece promotion_piece, Piece captured_piece)
    : from{from}, to{to}, piece{Piece::Pawn}, captured_piece{captured_piece}, promotion_piece{promotion_piece} {}

inline u64 Move::get_from() const { return from; }

inline u64 Move::get_to() const { return to; }

inline Piece Move::get_piece() const { return piece; }

inline Piece Move::get_captured_piece() const { return captured_piece; }

inline Piece Move::get_promotion_piece() const { return promotion_piece; }

inline bool Move::is_capture() const { return captured_piece != Piece::None; }

inline bool Move::is_promotion() const { return promotion_piece != Piece::None; }

inline bool Move::operator==(const Move& move) const {
  return from == move.from && to == move.to && piece == move.piece && captured_piece == move.captured_piece &&
         promotion_piece == move.promotion_piece;
}