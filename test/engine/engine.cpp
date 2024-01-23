#include "chess_engine/engine.h"

#include <iostream>
#include <string_view>

#include "chess/board.h"
#include "chess/move.h"
#include "doctest.h"

constexpr std::chrono::milliseconds ENGINE_SEARCH_TIME{1000};

Move choose_move_for_epd(std::string_view epd) {
  Board board = Board::from_epd(epd);
  Engine engine{};
  return engine.choose_move(board, ENGINE_SEARCH_TIME).move;
}

TEST_SUITE("checkmate") {
  TEST_CASE("mate in one") {
    Move move = choose_move_for_epd("6k1/6pp/1R1N1p2/p2r1P2/P7/2pn2P1/6KP/5R2 w - -");
    REQUIRE(move.to_uci() == "b6b8");
  }

  TEST_CASE("mate in two") {
    Move move = choose_move_for_epd("7Q/1r2k1pp/2b1p3/2q5/4pN2/P2n3P/1P1K2P1/R4B1R b - -");
    REQUIRE(move.to_uci() == "b7b2");
  }

  TEST_CASE("mate in three") {
    Move move = choose_move_for_epd("8/p4pkp/4r3/8/3P2pP/2P1q1P1/4Q3/5K1R b - -");
    REQUIRE(move.to_uci() == "e3e2");
  }

  TEST_CASE("mate in four") {
    Move move = choose_move_for_epd("8/8/1pk5/6n1/4P3/r2RK3/7q/8 b - -");
    REQUIRE(move.to_uci() == "h2g3");
  }

  TEST_CASE("mate in five") {
    Move move = choose_move_for_epd("2k5/pp2p3/4p3/4b2p/5RP1/1PpP3P/P1P3K1/4q3 b - -");
    REQUIRE(move.to_uci() == "e5f4");
  }

  TEST_CASE("mate in six") {
    Move move = choose_move_for_epd("8/4k3/4p1p1/2b1P2p/2P2P1P/5K2/p1r3r1/3RR3 b - -");
    REQUIRE(move.to_uci() == "c2f2");
  }
}

TEST_SUITE("hanging pieces") {
  TEST_CASE("engine captures free pawn") {
    Move move = choose_move_for_epd("rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq -");
    REQUIRE(move.to_uci() == "d4e5");
  }
}
