#include "player.h"

Player::Player(u64 bishop, u64 king, u64 knight, u64 pawn, u64 queen, u64 rook, bool can_castle_kingside,
               bool can_castle_queenside)
    : pieces{bishop, king, knight, pawn, queen, rook},
      can_castle_kingside_{can_castle_kingside},
      can_castle_queenside_{can_castle_queenside} {}

Player Player::white_initial() {
  return Player(bitboard::C1 | bitboard::F1, bitboard::E1, bitboard::B1 | bitboard::G1, bitboard::RANK_2, bitboard::D1,
                bitboard::A1 | bitboard::H1, true, true);
}

Player Player::black_initial() {
  return Player(bitboard::C8 | bitboard::F8, bitboard::E8, bitboard::B8 | bitboard::G8, bitboard::RANK_7, bitboard::D8,
                bitboard::A8 | bitboard::H8, true, true);
}

PieceVariant Player::piece_at(u64 bit) const {
  for (PieceVariant piece : {PieceVariant::Bishop, PieceVariant::King, PieceVariant::Knight, PieceVariant::Pawn,
                             PieceVariant::Queen, PieceVariant::Rook}) {
    if (pieces[static_cast<int>(piece)] & bit) return piece;
  }
  return PieceVariant::None;
}

Player& Player::operator&=(u64 mask) {
  pieces[static_cast<int>(PieceVariant::Bishop)] &= mask;
  pieces[static_cast<int>(PieceVariant::Knight)] &= mask;
  pieces[static_cast<int>(PieceVariant::Pawn)] &= mask;
  pieces[static_cast<int>(PieceVariant::Queen)] &= mask;
  pieces[static_cast<int>(PieceVariant::Rook)] &= mask;
  return *this;
}
