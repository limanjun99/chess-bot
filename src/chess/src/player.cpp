#include "player.h"

using namespace chess;

Player::Player(Bitboard bishop, Bitboard king, Bitboard knight, Bitboard pawn, Bitboard queen, Bitboard rook,
               bool can_castle_kingside, bool can_castle_queenside)
    : can_castle_kingside_{can_castle_kingside}, can_castle_queenside_{can_castle_queenside} {
  //! TODO: This is kinda ugly but necessary because the order of pieces depends on the value of PieceVariant::xxx.
  //! How to make this less error prone? (e.g. mistyping order of pieces, or when we change PieceVariant:: values)
  pieces[static_cast<size_t>(PieceVariant::Bishop)] = bishop;
  pieces[static_cast<size_t>(PieceVariant::King)] = king;
  pieces[static_cast<size_t>(PieceVariant::Knight)] = knight;
  pieces[static_cast<size_t>(PieceVariant::Pawn)] = pawn;
  pieces[static_cast<size_t>(PieceVariant::Queen)] = queen;
  pieces[static_cast<size_t>(PieceVariant::Rook)] = rook;
}

Player Player::empty() {
  return Player{bitboard::EMPTY, bitboard::EMPTY, bitboard::EMPTY, bitboard::EMPTY,
                bitboard::EMPTY, bitboard::EMPTY, false,           false};
}

Player Player::white_initial() {
  return Player(bitboard::C1 | bitboard::F1, bitboard::E1, bitboard::B1 | bitboard::G1, bitboard::RANK_2, bitboard::D1,
                bitboard::A1 | bitboard::H1, true, true);
}

Player Player::black_initial() {
  return Player(bitboard::C8 | bitboard::F8, bitboard::E8, bitboard::B8 | bitboard::G8, bitboard::RANK_7, bitboard::D8,
                bitboard::A8 | bitboard::H8, true, true);
}

PieceVariant Player::piece_at(Bitboard bit) const {
  for (PieceVariant piece : {PieceVariant::Bishop, PieceVariant::King, PieceVariant::Knight, PieceVariant::Pawn,
                             PieceVariant::Queen, PieceVariant::Rook}) {
    if (pieces[static_cast<int>(piece)] & bit) return piece;
  }
  return PieceVariant::None;
}

Player& Player::operator&=(Bitboard mask) {
  pieces[static_cast<int>(PieceVariant::Bishop)] &= mask;
  pieces[static_cast<int>(PieceVariant::Knight)] &= mask;
  pieces[static_cast<int>(PieceVariant::Pawn)] &= mask;
  pieces[static_cast<int>(PieceVariant::Queen)] &= mask;
  pieces[static_cast<int>(PieceVariant::Rook)] &= mask;
  return *this;
}
