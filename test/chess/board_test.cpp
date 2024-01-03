#include "chess/board.h"

#include <iostream>

#include "doctest.h"

TEST_CASE("kingside castling is applied to board properly") {
  Board board = Board::from_epd("rnbqk2r/ppppppbp/5np1/8/8/5NP1/PPPPPPBP/RNBQK2R w KQkq -");
  board = board.apply_uci_move("e1g1");
  REQUIRE(board.opp_player().get_bitboard(Piece::King) == bitboard::G1);
  REQUIRE(board.opp_player().get_bitboard(Piece::Rook) == (bitboard::A1 | bitboard::F1));
  board = board.apply_uci_move("e8g8");
  REQUIRE(board.opp_player().get_bitboard(Piece::King) == bitboard::G8);
  REQUIRE(board.opp_player().get_bitboard(Piece::Rook) == (bitboard::A8 | bitboard::F8));
}

TEST_CASE("queenside castling is applied to board properly") {
  Board board = Board::from_epd("r3kbnr/pbpqpppp/1pnp4/8/8/1PNP4/PBPQPPPP/R3KBNR w KQkq -");
  board = board.apply_uci_move("e1c1");
  REQUIRE(board.opp_player().get_bitboard(Piece::King) == bitboard::C1);
  REQUIRE(board.opp_player().get_bitboard(Piece::Rook) == (bitboard::D1 | bitboard::H1));
  board = board.apply_uci_move("e8c8");
  REQUIRE(board.opp_player().get_bitboard(Piece::King) == bitboard::C8);
  REQUIRE(board.opp_player().get_bitboard(Piece::Rook) == (bitboard::D8 | bitboard::H8));
}