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
  inline void disable_castling() { can_castle_kingside_ = can_castle_queenside_ = false; }

  // Not allowed to castle kingside anymore due to rook move.
  inline void disable_kingside_castling() { can_castle_kingside_ = false; }

  // Not allowed to castle queenside anymore due to rook move.
  inline void disable_queenside_castling() { can_castle_queenside_ = false; }

  // Enable both kingside and queenside castling.
  inline void enable_castling() { can_castle_kingside_ = can_castle_queenside_ = true; }

  // Enable kingside castling.
  inline void enable_kingside_castling() { can_castle_kingside_ = true; }

  // Enable queenside castling.
  inline void enable_queenside_castling() { can_castle_queenside_ = true; }

  // Returns true if the player is still allowed to castle kingside.
  inline u64 can_castle_kingside() const { return can_castle_kingside_; }

  // Returns true if the player is still allowed to castle queenside.
  inline u64 can_castle_queenside() const { return can_castle_queenside_; }

  // Returns a reference to the bitboard of the given piece.
  inline u64& operator[](Piece piece) { return pieces[static_cast<int>(piece)]; }
  inline const u64& operator[](Piece piece) const { return pieces[static_cast<int>(piece)]; }

  // Returns the piece at the given bit (or Piece::None if no piece is there).
  Piece piece_at(u64 bit) const;

  // Returns the bitwise OR of all pieces.
  u64 occupied() const;

  // Applies bitwise AND of the given mask to all pieces except the king.
  Player& operator&=(u64 mask);

private:
  u64 pieces[6];
  bool can_castle_kingside_;
  bool can_castle_queenside_;
};