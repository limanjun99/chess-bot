#include "chess/moveset.h"

#include <iostream>

#include "chess/bitboard.h"
#include "chess/board.h"
#include "chess/move.h"
#include "doctest.h"

TEST_CASE("moveset generation test - white castling") {
  // White cannot castle queenside due to the knight blocking it.
  Board board = Board::from_epd("r3k2r/pbpqppbp/np1p2p1/8/8/NPnP1NP1/PBPQPPBP/R3K2R w KQq -");
  MoveSet move_set = board.generate_moves();
  bool found_kingside_castling = false;
  bool found_queenside_castling = false;

  while (auto result = move_set.apply_next()) {
    auto &[move, _] = *result;
    if (move.get_piece() == Piece::King) {
      found_kingside_castling |= move.get_from() == bitboard::E1 && move.get_to() == bitboard::G1;
      found_queenside_castling |= move.get_from() == bitboard::E1 && move.get_to() == bitboard::C1;
    }
  }

  REQUIRE(found_kingside_castling == true);
  REQUIRE(found_queenside_castling == false);
}

TEST_CASE("moveset generation test - black castling") {
  // Black cannot castle kingside as the rook already moved.
  Board board = Board::from_epd("r3k2r/pbpqppbp/np1p2p1/8/8/NPnP1NP1/PBPQPPBP/R3K2R b KQq -");
  MoveSet move_set = board.generate_moves();
  bool found_kingside_castling = false;
  bool found_queenside_castling = false;

  while (auto result = move_set.apply_next()) {
    auto &[move, _] = *result;
    if (move.get_piece() == Piece::King) {
      found_kingside_castling |= move.get_from() == bitboard::E8 && move.get_to() == bitboard::G8;
      found_queenside_castling |= move.get_from() == bitboard::E8 && move.get_to() == bitboard::C8;
    }
  }

  REQUIRE(found_kingside_castling == false);
  REQUIRE(found_queenside_castling == true);
}

TEST_CASE("moveset generation test - en passant") {
  Board board = Board::from_epd("rnbqkbnr/1pp1ppp1/p7/2PpP2p/8/8/PP1P1PPP/RNBQKBNR w KQkq d6");
  MoveSet move_set = board.generate_moves();
  bool found_c5_en_passant = false;
  bool found_e5_en_passant = false;

  while (auto result = move_set.apply_next()) {
    auto &[move, _] = *result;
    if (move.get_piece() == Piece::Pawn) {
      found_c5_en_passant |= move.get_from() == bitboard::C5 && move.get_to() == bitboard::D6;
      found_e5_en_passant |= move.get_from() == bitboard::E5 && move.get_to() == bitboard::D6;
    }
  }

  REQUIRE(found_c5_en_passant);
  REQUIRE(found_e5_en_passant);
}