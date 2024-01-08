#include "chess/board.h"

#include <iostream>

#include "chess/moveset.h"
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

TEST_CASE("move generation from initial state is correct") {
  Board board = Board::initial();
  MoveSet move_set = board.generate_moves();
  int move_count[6] = {0};
  while (auto result = move_set.apply_next()) {
    auto& [move, _] = *result;
    move_count[static_cast<int>(move.get_piece())]++;
  }
  REQUIRE(move_count[static_cast<int>(Piece::Bishop)] == 0);
  REQUIRE(move_count[static_cast<int>(Piece::King)] == 0);
  REQUIRE(move_count[static_cast<int>(Piece::Knight)] == 4);
  REQUIRE(move_count[static_cast<int>(Piece::Pawn)] == 16);
  REQUIRE(move_count[static_cast<int>(Piece::Queen)] == 0);
  REQUIRE(move_count[static_cast<int>(Piece::Rook)] == 0);
}

TEST_CASE("pawn push does not wrongly capture en passant") {
  Board board = Board::initial();
  board = board.apply_uci_move("d2d4");
  board = board.apply_uci_move("e7e5");
  // Before fixing, en passant was handled wrongly and caused e2e4 to capture the pawn on e5.
  board = board.apply_uci_move("e2e4");
  REQUIRE((board.cur_player().get_bitboard(Piece::Pawn) & bitboard::E5) == bitboard::E5);
}

TEST_CASE("moving one rook only disables castling for that side") {
  Board board = Board::initial();
  board = board.apply_uci_move("h2h3");
  board = board.apply_uci_move("h7h6");
  board = board.apply_uci_move("h1h2");
  REQUIRE(!board.opp_player().can_castle_kingside());
  REQUIRE(board.opp_player().can_castle_queenside());
  board = board.apply_uci_move("h8h7");
  REQUIRE(!board.opp_player().can_castle_kingside());
  REQUIRE(board.opp_player().can_castle_queenside());
}

TEST_CASE("promoting a pawn should remove the pawn from the board") {
  Board board = Board::from_epd("rnbqkbnr/1pppppPp/8/8/8/p7/PPPPPPP1/RNBQKBNR w KQkq -");
  board = board.apply_uci_move("g7g8q");
  REQUIRE((board.opp_player().get_bitboard(Piece::Pawn) & bitboard::G8) == 0);
}