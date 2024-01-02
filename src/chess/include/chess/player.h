#pragma once

#include "bitboard.h"
#include "piece.h"

// Stores the bitboards for each piece, and whether the player can still castle.
class Player {
public:
  Player(u64 bishop, u64 king, u64 knight, u64 pawn, u64 rook, u64 queen, bool can_castle_kingside,
         bool can_castle_queenside);

  // Starting state for white.
  static Player white_initial();
  // Starting state for black.
  static Player black_initial();

  // Not allowed to castle anymore due to king move.
  void disable_castling();

  // Not allowed to castle kingside anymore due to rook move.
  void disable_kingside_castling();

  // Not allowed to castle queenside anymore due to rook move.
  void disable_queenside_castling();

  // Enable both kingside and queenside castling.
  void enable_castling();

  // Enable kingside castling.
  void enable_kingside_castling();

  // Enable queenside castling.
  void enable_queenside_castling();

  // Returns true if the player is still allowed to castle kingside.
  u64 can_castle_kingside() const;

  // Returns true if the player is still allowed to castle queenside.
  u64 can_castle_queenside() const;

  // Returns the bitboard of the given piece.
  u64 get_bitboard(Piece piece) const;

  // Returns a mutable reference to a piece's bitboard.
  u64& mut_bitboard(Piece piece);

  // Returns the bitwise OR of all pieces.
  u64 occupied() const;

  // Applies bitwise AND of the given mask to all pieces except the king.
  Player& operator&=(u64 mask);

private:
  u64 pieces[6];
  bool can_castle_kingside_;
  bool can_castle_queenside_;
};