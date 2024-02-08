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
  bool can_castle_kingside() const;

  // Returns true if the player is still allowed to castle queenside.
  bool can_castle_queenside() const;

  // Returns a reference to the bitboard of the given piece.
  u64& operator[](PieceVariant piece);
  const u64& operator[](PieceVariant piece) const;

  // Returns the piece at the given bit (or PieceVariant::None if no piece is there).
  PieceVariant piece_at(u64 bit) const;

  // Returns the bitwise OR of all pieces.
  u64 occupied() const;

  // Applies bitwise AND of the given mask to all pieces except the king.
  Player& operator&=(u64 mask);

private:
  u64 pieces[6];
  bool can_castle_kingside_;
  bool can_castle_queenside_;
};

inline void Player::disable_castling() { can_castle_kingside_ = can_castle_queenside_ = false; }

inline void Player::disable_kingside_castling() { can_castle_kingside_ = false; }

inline void Player::disable_queenside_castling() { can_castle_queenside_ = false; }

inline void Player::enable_castling() { can_castle_kingside_ = can_castle_queenside_ = true; };

inline void Player::enable_kingside_castling() { can_castle_kingside_ = true; }

inline void Player::enable_queenside_castling() { can_castle_queenside_ = true; }

inline bool Player::can_castle_kingside() const { return can_castle_kingside_; }

inline bool Player::can_castle_queenside() const { return can_castle_queenside_; }

inline u64& Player::operator[](PieceVariant piece) { return pieces[static_cast<int>(piece)]; }

inline const u64& Player::operator[](PieceVariant piece) const { return pieces[static_cast<int>(piece)]; }

inline u64 Player::occupied() const { return pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5]; }