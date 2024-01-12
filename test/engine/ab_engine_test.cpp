#include "chess_engine/ab_engine.h"

#include <iostream>

#include "chess/board.h"
#include "doctest.h"

TEST_CASE("engine captures free piece") {
  Board board = Board::initial();
  board = board.apply_uci_move("d2d4");
  board = board.apply_uci_move("e7e5");
  AlphaBetaEngine engine{0};
  Move move = engine.make_move(board).move;
  REQUIRE(move.to_uci() == "d4e5");
}

TEST_CASE("engine finds mate in one") {
  Board board = Board::from_epd("k7/pp4Qp/2R2p2/4B3/7P/N3p3/PP3P2/3K2NR w - -");
  AlphaBetaEngine engine{5};
  Move move = engine.make_move(board).move;
  board = board.apply_move(move.get_piece(), move.get_from(), move.get_to());
  REQUIRE(board.is_in_check());  // Any check here is a checkmate
}