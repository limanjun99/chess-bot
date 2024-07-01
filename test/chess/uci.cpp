#include "chess/uci.h"

#include <iostream>

#include "chess/board.h"
#include "chess/move.h"
#include "doctest.h"

using namespace chess;

TEST_SUITE("uci") {
  TEST_CASE("uci::move works for normal move") {
    const Board board{Board::initial()};
    const Move move{uci::move("a2a3", board)};
    REQUIRE(move.get_from() == Bitboard::A2);
    REQUIRE(move.get_to() == Bitboard::A3);
    REQUIRE(move.get_piece() == PieceType::Pawn);
    REQUIRE(move.get_captured_piece() == PieceType::None);
  }

  TEST_CASE("uci::move works for captures") {
    const Board board{Board::from_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2")};
    const Move move{uci::move("e4d5", board)};
    REQUIRE(move.get_from() == Bitboard::E4);
    REQUIRE(move.get_to() == Bitboard::D5);
    REQUIRE(move.get_piece() == PieceType::Pawn);
    REQUIRE(move.get_captured_piece() == PieceType::Pawn);
  }

  TEST_CASE("uci::move works for promotions") {
    const Board board{Board::from_fen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1")};
    const Move move{uci::move("a7a8n", board)};
    REQUIRE(move.get_from() == Bitboard::A7);
    REQUIRE(move.get_to() == Bitboard::A8);
    REQUIRE(move.get_piece() == PieceType::Pawn);
    REQUIRE(move.get_captured_piece() == PieceType::None);
    REQUIRE(move.get_promotion_piece() == PieceType::Knight);
  }
}