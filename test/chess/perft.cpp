#include <functional>
#include <iostream>

#include "chess/board.h"
#include "chess/move.h"
#include "doctest.h"

// Perft is a move generation test that checks that the number of nodes in a search tree at each depth are correct.
// This file contains testcases taken from https://www.chessprogramming.org/Perft_Results

void search(std::vector<int>& node_count, const Board& board, size_t depth) {
  node_count[depth]++;
  if (depth + 1 >= node_count.size()) return;
  for (auto& move : board.generate_moves()) {
    search(node_count, board.apply_move(move), depth + 1);
  }
}

TEST_CASE("perft initial position") {
  const Board board = Board::initial();
  const std::vector<int> correct_node_count = {1, 20, 400, 8902, 197281, 4865609};
  std::vector<int> node_count(correct_node_count.size(), 0);
  search(node_count, board, 0);
  for (size_t i = 0; i < node_count.size(); i++) REQUIRE(node_count[i] == correct_node_count[i]);
}

TEST_CASE("perft position 2") {
  const Board board = Board::from_epd("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  const std::vector<int> correct_node_count = {1, 48, 2039, 97862, 4085603};
  std::vector<int> node_count(correct_node_count.size(), 0);
  search(node_count, board, 0);
  for (size_t i = 0; i < node_count.size(); i++) REQUIRE(node_count[i] == correct_node_count[i]);
}

TEST_CASE("perft position 3") {
  const Board board = Board::from_epd("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
  const std::vector<int> correct_node_count = {1, 14, 191, 2812, 43238, 674624, 11030083};
  std::vector<int> node_count(correct_node_count.size(), 0);
  search(node_count, board, 0);
  for (size_t i = 0; i < node_count.size(); i++) REQUIRE(node_count[i] == correct_node_count[i]);
}

TEST_CASE("perft position 4") {
  const Board board = Board::from_epd("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -");
  const std::vector<int> correct_node_count = {1, 6, 264, 9467, 422333, 15833292};
  std::vector<int> node_count(correct_node_count.size(), 0);
  search(node_count, board, 0);
  for (size_t i = 0; i < node_count.size(); i++) REQUIRE(node_count[i] == correct_node_count[i]);
}

TEST_CASE("perft position 5") {
  const Board board = Board::from_epd("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -");
  const std::vector<int> correct_node_count = {1, 44, 1486, 62379, 2103487, 89941194};
  std::vector<int> node_count(correct_node_count.size(), 0);
  search(node_count, board, 0);
  for (size_t i = 0; i < node_count.size(); i++) REQUIRE(node_count[i] == correct_node_count[i]);
}

TEST_CASE("perft position 6") {
  const Board board = Board::from_epd("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -");
  const std::vector<int> correct_node_count = {1, 46, 2079, 89890, 3894594};
  std::vector<int> node_count(correct_node_count.size(), 0);
  search(node_count, board, 0);
  for (size_t i = 0; i < node_count.size(); i++) REQUIRE(node_count[i] == correct_node_count[i]);
}

void search_quiescence(const Board& board, size_t depth, size_t max_depth) {
  if (depth >= max_depth) return;
  auto moves = board.generate_moves();
  if (!board.is_in_check()) {
    auto captures_and_promotions = board.generate_quiescence_moves();
    auto captures_checks_and_promotions = board.generate_quiescence_moves_and_checks();

    // Verify that correct number of quiescence moves are generated.
    unsigned int captures_and_promotions_cnt = 0;
    unsigned int captures_checks_and_promotions_cnt = 0;
    for (auto& move : moves) {
      if (board.is_a_capture(move) || move.is_promotion()) {
        captures_and_promotions_cnt++;
        captures_checks_and_promotions_cnt++;
      } else if (board.is_a_check(move)) {
        captures_checks_and_promotions_cnt++;
      }
    }
    REQUIRE(captures_and_promotions_cnt == captures_and_promotions.size());
    REQUIRE(captures_checks_and_promotions_cnt == captures_checks_and_promotions.size());
  }

  for (auto& move : moves) {
    search_quiescence(board.apply_move(move), depth + 1, max_depth);
  }
}

TEST_CASE("perft initial position - quiescence") {
  const Board board = Board::initial();
  search_quiescence(board, 0, 5);
}

TEST_CASE("perft position 2 - quiescence") {
  const Board board = Board::from_epd("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  search_quiescence(board, 0, 4);
}

TEST_CASE("perft position 3 - quiescence") {
  const Board board = Board::from_epd("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
  search_quiescence(board, 0, 6);
}

TEST_CASE("perft position 4 - quiescence") {
  const Board board = Board::from_epd("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -");
  search_quiescence(board, 0, 5);
}

TEST_CASE("perft position 5 - quiescence") {
  const Board board = Board::from_epd("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -");
  search_quiescence(board, 0, 5);
}

TEST_CASE("perft position 6 - quiescence") {
  const Board board = Board::from_epd("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -");
  search_quiescence(board, 0, 4);
}