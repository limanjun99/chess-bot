#include "chess_engine/naive_engine.h"

#include <iostream>

#include "chess/board.h"
#include "doctest.h"

TEST_CASE("engine captures free piece") {
  Board board = Board::initial();
  board = board.apply_uci_move("d2d4");
  board = board.apply_uci_move("e7e5");
  NaiveEngine engine{0};
  Move move = engine.make_move(board);
  REQUIRE(move.to_uci() == "d4e5");
}