#pragma once

#include <array>

#include "bitboard.h"
#include "piece.h"

namespace chess {

// Stores the bitboards for each piece, and whether the player can still castle.
class Player {
public:
  constexpr explicit Player(Bitboard bishop, Bitboard king, Bitboard knight, Bitboard pawn, Bitboard rook,
                            Bitboard queen, bool can_castle_kingside, bool can_castle_queenside);

  // A player with no pieces and no castling rights.
  static constexpr Player empty();
  // Starting state for white.
  static constexpr Player white_initial();
  // Starting state for black.
  static constexpr Player black_initial();

  // Not allowed to castle anymore due to king move.
  constexpr void disable_castling();

  // Not allowed to castle kingside anymore due to rook move.
  constexpr void disable_kingside_castling();

  // Not allowed to castle queenside anymore due to rook move.
  constexpr void disable_queenside_castling();

  // Enable both kingside and queenside castling.
  constexpr void enable_castling();

  // Enable kingside castling.
  constexpr void enable_kingside_castling();

  // Enable queenside castling.
  constexpr void enable_queenside_castling();

  // Returns true if the player is still allowed to castle kingside.
  constexpr bool can_castle_kingside() const;

  // Returns true if the player is still allowed to castle queenside.
  constexpr bool can_castle_queenside() const;

  // Returns a reference to the bitboard of the given piece.
  constexpr Bitboard& operator[](PieceType piece);
  constexpr const Bitboard& operator[](PieceType piece) const;

  // Returns the piece at the given bit (or PieceType::None if no piece is there).
  constexpr PieceType piece_at(Bitboard bit) const;

  // Returns the bitwise OR of all pieces.
  constexpr Bitboard occupied() const;

  // Applies bitwise AND of the given mask to all pieces except the king.
  constexpr Player& operator&=(Bitboard mask);

private:
  std::array<Bitboard, 6> pieces;
  bool can_castle_kingside_;
  bool can_castle_queenside_;
};

// ========== IMPLEMENTATIONS ==========

constexpr Player::Player(Bitboard bishop, Bitboard king, Bitboard knight, Bitboard pawn, Bitboard queen, Bitboard rook,
                         bool can_castle_kingside, bool can_castle_queenside)
    : can_castle_kingside_{can_castle_kingside}, can_castle_queenside_{can_castle_queenside} {
  //! TODO: This is kinda ugly but necessary because the order of pieces depends on the value of PieceType::xxx.
  //! How to make this less error prone? (e.g. mistyping order of pieces, or when we change PieceType:: values)
  pieces[static_cast<size_t>(PieceType::Bishop)] = bishop;
  pieces[static_cast<size_t>(PieceType::King)] = king;
  pieces[static_cast<size_t>(PieceType::Knight)] = knight;
  pieces[static_cast<size_t>(PieceType::Pawn)] = pawn;
  pieces[static_cast<size_t>(PieceType::Queen)] = queen;
  pieces[static_cast<size_t>(PieceType::Rook)] = rook;
}

constexpr Player Player::empty() {
  return Player{Bitboard::empty, Bitboard::empty, Bitboard::empty, Bitboard::empty,
                Bitboard::empty, Bitboard::empty, false,           false};
}

constexpr Player Player::white_initial() {
  return Player(Bitboard::C1 | Bitboard::F1, Bitboard::E1, Bitboard::B1 | Bitboard::G1, Bitboard::rank_2, Bitboard::D1,
                Bitboard::A1 | Bitboard::H1, true, true);
}

constexpr Player Player::black_initial() {
  return Player(Bitboard::C8 | Bitboard::F8, Bitboard::E8, Bitboard::B8 | Bitboard::G8, Bitboard::rank_7, Bitboard::D8,
                Bitboard::A8 | Bitboard::H8, true, true);
}

constexpr void Player::disable_castling() { can_castle_kingside_ = can_castle_queenside_ = false; }

constexpr void Player::disable_kingside_castling() { can_castle_kingside_ = false; }

constexpr void Player::disable_queenside_castling() { can_castle_queenside_ = false; }

constexpr void Player::enable_castling() { can_castle_kingside_ = can_castle_queenside_ = true; };

constexpr void Player::enable_kingside_castling() { can_castle_kingside_ = true; }

constexpr void Player::enable_queenside_castling() { can_castle_queenside_ = true; }

constexpr bool Player::can_castle_kingside() const { return can_castle_kingside_; }

constexpr bool Player::can_castle_queenside() const { return can_castle_queenside_; }

constexpr Bitboard& Player::operator[](PieceType piece) { return pieces[static_cast<size_t>(piece)]; }

constexpr const Bitboard& Player::operator[](PieceType piece) const { return pieces[static_cast<size_t>(piece)]; }

constexpr PieceType Player::piece_at(Bitboard bit) const {
  for (size_t type{0}; type < pieces.size(); type++) {
    if (pieces[type] & bit) return static_cast<PieceType>(type);
  }
  return PieceType::None;
}

constexpr Bitboard Player::occupied() const {
  return std::reduce(pieces.begin(), pieces.end(), Bitboard::empty, std::bit_or{});
}

constexpr Player& Player::operator&=(Bitboard mask) {
  operator[](PieceType::Bishop) &= mask;
  operator[](PieceType::Knight) &= mask;
  operator[](PieceType::Pawn) &= mask;
  operator[](PieceType::Queen) &= mask;
  operator[](PieceType::Rook) &= mask;
  return *this;
}

}  // namespace chess