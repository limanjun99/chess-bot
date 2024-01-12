#pragma once

#include <optional>
#include <string>

#include "bitboard.h"
#include "piece.h"

class Move {
public:
  inline Move() {}

  // Construct a move of the given piece.
  inline Move(Piece piece, u64 from, u64 to) : from{from}, to{to}, piece{piece}, promotion_piece{Piece::None} {}

  // Construct a promotion to the given promotion_piece.
  inline Move(u64 from, u64 to, Piece promotion_piece)
      : from{from}, to{to}, piece{Piece::Pawn}, promotion_piece{promotion_piece} {}

  // Getter for from bitboard.
  inline u64 get_from() const { return from; }
  // Getter for to bitboard.
  inline u64 get_to() const { return to; }
  // Getter for moved piece.
  inline Piece get_piece() const { return piece; }
  // Getter for promotion piece.
  inline Piece get_promotion_piece() const { return promotion_piece; }

  // Whether this move is a promotion.
  inline bool is_promotion() const { return promotion_piece != Piece::None; }

  // Returns this move in UCI format.
  std::string to_uci() const;

private:
  u64 from;
  u64 to;
  Piece piece;
  Piece promotion_piece;
};