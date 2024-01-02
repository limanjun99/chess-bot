#include "player.h"

Player::Player(u64 bishop, u64 king, u64 knight, u64 pawn, u64 rook, u64 queen, bool can_castle_kingside,
               bool can_castle_queenside)
    : pieces{bishop, king, knight, pawn, rook, queen},
      can_castle_kingside_{can_castle_kingside},
      can_castle_queenside_{can_castle_queenside} {}

Player Player::white_initial() {
  return Player(bitboard::C1 | bitboard::F1, bitboard::E1, bitboard::B1 | bitboard::G1, bitboard::RANK_2,
                bitboard::A1 | bitboard::H1, bitboard::D1, true, true);
}

Player Player::black_initial() {
  return Player(bitboard::C1 | bitboard::F1, bitboard::E1, bitboard::B1 | bitboard::G1, bitboard::RANK_2,
                bitboard::A1 | bitboard::H1, bitboard::D1, true, true);
}

void Player::disable_castling() {
  can_castle_kingside_ = false;
  can_castle_queenside_ = false;
}

void Player::disable_kingside_castling() { can_castle_kingside_ = false; }

void Player::disable_queenside_castling() { can_castle_queenside_ = false; }

void Player::enable_castling() {
  can_castle_kingside_ = true;
  can_castle_queenside_ = true;
}

void Player::enable_kingside_castling() { can_castle_kingside_ = true; }

void Player::enable_queenside_castling() { can_castle_queenside_ = true; }

u64 Player::can_castle_kingside() const { return can_castle_kingside_; }

u64 Player::can_castle_queenside() const { return can_castle_queenside_; }

u64 Player::get_bitboard(Piece piece) const { return pieces[static_cast<int>(piece)]; }

u64& Player::mut_bitboard(Piece piece) { return pieces[static_cast<int>(piece)]; }

u64 Player::occupied() const { return pieces[0] | pieces[1] | pieces[2] | pieces[3] | pieces[4] | pieces[5]; }

Player& Player::operator&=(u64 mask) {
  pieces[static_cast<int>(Piece::Bishop)] &= mask;
  pieces[static_cast<int>(Piece::Knight)] &= mask;
  pieces[static_cast<int>(Piece::Pawn)] &= mask;
  pieces[static_cast<int>(Piece::Queen)] &= mask;
  pieces[static_cast<int>(Piece::Rook)] &= mask;
  return *this;
}
