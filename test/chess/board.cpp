#include "chess/board.h"

#include <iostream>

#include "doctest.h"

TEST_SUITE("board.from_epd") {
  TEST_CASE("castling rights") {
    auto board = Board::from_epd("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
    REQUIRE(board.get_white().can_castle_kingside());
    REQUIRE(board.get_white().can_castle_queenside());
    REQUIRE(board.get_black().can_castle_kingside());
    REQUIRE(board.get_black().can_castle_queenside());
    REQUIRE(board.get_en_passant() == 0);
  }

  TEST_CASE("no castling rights") {
    auto board = Board::from_epd("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - -");
    REQUIRE(!board.get_white().can_castle_kingside());
    REQUIRE(!board.get_white().can_castle_queenside());
    REQUIRE(!board.get_black().can_castle_kingside());
    REQUIRE(!board.get_black().can_castle_queenside());
    REQUIRE(board.get_en_passant() == 0);
  }

  TEST_CASE("en passant with castling") {
    auto board = Board::from_epd("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6");
    REQUIRE(board.get_white().can_castle_kingside());
    REQUIRE(board.get_white().can_castle_queenside());
    REQUIRE(board.get_black().can_castle_kingside());
    REQUIRE(board.get_black().can_castle_queenside());
    REQUIRE(board.get_en_passant() == bitboard::D6);
  }

  TEST_CASE("en passant without castling") {
    auto board = Board::from_epd("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w - d6");
    REQUIRE(!board.get_white().can_castle_kingside());
    REQUIRE(!board.get_white().can_castle_queenside());
    REQUIRE(!board.get_black().can_castle_kingside());
    REQUIRE(!board.get_black().can_castle_queenside());
    REQUIRE(board.get_en_passant() == bitboard::D6);
  }
}