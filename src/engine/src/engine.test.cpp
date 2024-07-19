#include "chess_engine/engine.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <string_view>
#include <utility>

#include "chess/board.h"
#include "chess/move.h"
#include "chess_engine/uci.h"

chess::Move choose_move_for_fen(std::string_view fen, int depth) {
  const chess::Board board{chess::Board::from_fen(fen)};
  Engine engine{board};
  return engine.choose_move(depth).move;
}

TEST(Checkmate, MateInOne) {
  chess::Move move = choose_move_for_fen("6k1/6pp/1R1N1p2/p2r1P2/P7/2pn2P1/6KP/5R2 w - - 0 0", 2);
  EXPECT_EQ(move.to_uci(), "b6b8");
}

TEST(Checkmate, MateInTwo) {
  chess::Move move = choose_move_for_fen("7Q/1r2k1pp/2b1p3/2q5/4pN2/P2n3P/1P1K2P1/R4B1R b - - 0 0", 4);
  EXPECT_EQ(move.to_uci(), "b7b2");
}

TEST(CheckMate, MateInThree) {
  chess::Move move = choose_move_for_fen("8/p4pkp/4r3/8/3P2pP/2P1q1P1/4Q3/5K1R b - - 0 0", 6);
  EXPECT_EQ(move.to_uci(), "e3e2");
}

TEST(CheckMate, MateInFour) {
  chess::Move move = choose_move_for_fen("8/8/1pk5/6n1/4P3/r2RK3/7q/8 b - - 0 0", 8);
  EXPECT_EQ(move.to_uci(), "h2g3");
}

TEST(CheckMate, MateInFive) {
  chess::Move move = choose_move_for_fen("2k5/pp2p3/4p3/4b2p/5RP1/1PpP3P/P1P3K1/4q3 b - - 0 0", 10);
  EXPECT_EQ(move.to_uci(), "e5f4");
}

TEST(CheckMate, MateInSix) {
  chess::Move move = choose_move_for_fen("8/4k3/4p1p1/2b1P2p/2P2P1P/5K2/p1r3r1/3RR3 b - - 0 0", 12);
  EXPECT_EQ(move.to_uci(), "c2f2");
}

TEST(HangingPieces, FreePawn) {
  chess::Move move = choose_move_for_fen("rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 0", 10);
  EXPECT_EQ(move.to_uci(), "d4e5");
}

// The QuietPositions test suite contains positions taken from games the engine has played, where there is a single
// superior move, but the engine (in prior versions) failed to find it.

TEST(QuietPositions, Position1) {
  chess::Move move = choose_move_for_fen("3r1rk1/5ppp/p1bq4/P2pb3/6P1/1R3N1P/1P2NP2/3QK2R w K - 0 0", 9);
  EXPECT_EQ(move.to_uci(), "f3e5");
}

TEST(QuietPositions, Position2) {
  chess::Move move = choose_move_for_fen("2kr3r/ppp2ppp/2p1b3/8/1b1NP3/2N4P/PPP2PP1/R3K2R w KQ - 0 0", 9);
  EXPECT_EQ(move.to_uci(), "d4e6");
}

TEST(QuietPositions, Position3) {
  chess::Move move = choose_move_for_fen("5rk1/1ppb1pp1/1r1p1q1p/p1n5/P1PnPP2/RP1P1N2/3Q3P/3B1KNR w - - 0 0", 11);
  EXPECT_EQ(move.to_uci(), "d2c3");
}

//! TODO: Add back this testcase when it can be handled.
// TEST(QuietPositions, Position4) {
//   chess::Move move = choose_move_for_fen("2r2r2/1Q2k1p1/p2p2q1/7p/P3Np2/7P/6P1/5R1K b - - 0 0", 15);
//   EXPECT_EQ(move.to_uci(), "e7d8");
// }

// The TimeManagement test suite tests that the engine finishes within 1~2ms before the deadline.
// Finish before 1ms for some leeway, but at most 2ms to minimize wasted time.

void expect_time_management(chess::Board position, std::chrono::milliseconds movetime) {
  Engine engine{position};
  auto search_config{engine::uci::SearchConfig::Builder{}.set_movetime(movetime).build()};

  const auto start_time{std::chrono::steady_clock::now()};
  std::shared_ptr<Engine::SearchControl> search_control{engine.cancellable_search(std::move(search_config))};
  std::ignore = search_control->wait_and_get_move();
  const auto end_time{std::chrono::steady_clock::now()};
  const std::chrono::duration<double, std::milli> time_taken{end_time - start_time};

  const std::string error_message{std::format("Took {} searching with movetime {}", time_taken, movetime)};
  EXPECT_LE(time_taken, movetime - std::chrono::milliseconds{1}) << error_message;
  EXPECT_GE(time_taken, movetime - std::chrono::milliseconds{2}) << error_message;
}

TEST(TimeManagement, Control1) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{5}); }
TEST(TimeManagement, Control2) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{10}); }
TEST(TimeManagement, Control3) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{25}); }
TEST(TimeManagement, Control4) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{50}); }
TEST(TimeManagement, Control5) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{100}); }
TEST(TimeManagement, Control6) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{200}); }
TEST(TimeManagement, Control7) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{500}); }
TEST(TimeManagement, Control8) { expect_time_management(chess::Board::initial(), std::chrono::milliseconds{1000}); }