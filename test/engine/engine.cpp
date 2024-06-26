#include "chess_engine/engine.h"

#include <iostream>
#include <string_view>

#include "chess/board.h"
#include "chess/move.h"
#include "doctest.h"

constexpr std::chrono::milliseconds ENGINE_SEARCH_TIME{2000};

chess::Move choose_move_for_fen(std::string_view fen) {
  chess::Board board = chess::Board::from_fen(fen);
  Engine engine{board};
  return engine.choose_move(ENGINE_SEARCH_TIME).move;
}

chess::Move choose_move_for_fen(std::string_view fen, int depth) {
  chess::Board board = chess::Board::from_fen(fen);
  Engine engine{board};
  return engine.choose_move(depth).move;
}

TEST_SUITE("checkmate") {
  TEST_CASE("mate in one") {
    chess::Move move = choose_move_for_fen("6k1/6pp/1R1N1p2/p2r1P2/P7/2pn2P1/6KP/5R2 w - - 0 0");
    REQUIRE(move.to_uci() == "b6b8");
  }

  TEST_CASE("mate in two") {
    chess::Move move = choose_move_for_fen("7Q/1r2k1pp/2b1p3/2q5/4pN2/P2n3P/1P1K2P1/R4B1R b - - 0 0");
    REQUIRE(move.to_uci() == "b7b2");
  }

  TEST_CASE("mate in three") {
    chess::Move move = choose_move_for_fen("8/p4pkp/4r3/8/3P2pP/2P1q1P1/4Q3/5K1R b - - 0 0");
    REQUIRE(move.to_uci() == "e3e2");
  }

  TEST_CASE("mate in four") {
    chess::Move move = choose_move_for_fen("8/8/1pk5/6n1/4P3/r2RK3/7q/8 b - - 0 0");
    REQUIRE(move.to_uci() == "h2g3");
  }

  TEST_CASE("mate in five") {
    chess::Move move = choose_move_for_fen("2k5/pp2p3/4p3/4b2p/5RP1/1PpP3P/P1P3K1/4q3 b - - 0 0");
    REQUIRE(move.to_uci() == "e5f4");
  }

  TEST_CASE("mate in six") {
    chess::Move move = choose_move_for_fen("8/4k3/4p1p1/2b1P2p/2P2P1P/5K2/p1r3r1/3RR3 b - - 0 0");
    REQUIRE(move.to_uci() == "c2f2");
  }
}

TEST_SUITE("hanging pieces") {
  TEST_CASE("engine captures free pawn") {
    chess::Move move = choose_move_for_fen("rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 0");
    REQUIRE(move.to_uci() == "d4e5");
  }
}

TEST_SUITE("quiet positions") {
  // This test suite contains positions taken from games the engine has played, where there is a single superior move,
  // but the engine failed to find it.

  TEST_CASE("position 1") {
    chess::Move move = choose_move_for_fen("3r1rk1/5ppp/p1bq4/P2pb3/6P1/1R3N1P/1P2NP2/3QK2R w K - 0 0", 10);
    REQUIRE(move.to_uci() == "f3e5");
  }

  TEST_CASE("position 2") {
    chess::Move move = choose_move_for_fen("2kr3r/ppp2ppp/2p1b3/8/1b1NP3/2N4P/PPP2PP1/R3K2R w KQ - 0 0", 10);
    REQUIRE(move.to_uci() == "d4e6");
  }

  TEST_CASE("position 3") {
    chess::Move move = choose_move_for_fen("5rk1/1ppb1pp1/1r1p1q1p/p1n5/P1PnPP2/RP1P1N2/3Q3P/3B1KNR w - - 0 0", 12);
    REQUIRE(move.to_uci() == "d2c3");
  }

  TEST_CASE("position 4") {
    chess::Move move = choose_move_for_fen("2r2r2/1Q2k1p1/p2p2q1/7p/P3Np2/7P/6P1/5R1K b - - 0 0", 15);
    REQUIRE(move.to_uci() == "e7d8");
  }
}