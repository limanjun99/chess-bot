#include <benchmark/benchmark.h>

#include "chess/board.h"

using namespace chess;

void search(const Board& board, size_t current_depth) {
  if (current_depth <= 0) return;
  auto moves = board.generate_moves();
  for (const auto& move : moves) {
    search(board.apply_move(move), current_depth - 1);
  }
}

static void perft_position_1(benchmark::State& state) {
  Board board = Board::initial();
  for (auto _ : state) {
    search(board, 6);
  }
}
BENCHMARK(perft_position_1);

static void perft_position_2(benchmark::State& state) {
  Board board = Board::from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_2);

static void perft_position_3(benchmark::State& state) {
  Board board = Board::from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0");
  for (auto _ : state) {
    search(board, 7);
  }
}
BENCHMARK(perft_position_3);

static void perft_position_4(benchmark::State& state) {
  Board board = Board::from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq  0 0");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_4);

static void perft_position_5(benchmark::State& state) {
  Board board = Board::from_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 0");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_5);

static void perft_position_6(benchmark::State& state) {
  Board board = Board::from_fen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 0");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_6);