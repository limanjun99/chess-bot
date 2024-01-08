#pragma once

#include <optional>
#include <string>

#include "bitboard.h"
#include "piece.h"

class Move {
public:
  // Construct a move of the given piece.
  Move(Piece piece, u64 from, u64 to);

  // Construct a promotion to the given promotion_piece.
  Move(u64 from, u64 to, Piece promotion_piece);

  // Getter for from bitboard.
  inline u64 get_from() { return from; }
  // Getter for to bitboard.
  inline u64 get_to() { return to; }
  // Getter for moved piece.
  inline Piece get_piece() { return piece; }
  // Getter for promotion piece.
  inline std::optional<Piece> get_promotion_piece() { return promotion_piece; }

  // Whether this move is a promotion.
  inline bool is_promotion() { return promotion_piece.has_value(); }

  // Returns this move in UCI format.
  std::string to_uci() const;

private:
  Piece piece;
  u64 from;
  u64 to;
  std::optional<Piece> promotion_piece;
};