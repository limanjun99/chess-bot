#include "chess/board.h"

#include <benchmark/benchmark.h>

using namespace chess;

static void board_initial_position_move_generation(benchmark::State& state) {
  Board board = Board::initial();
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_initial_position_move_generation);

static void board_middlegame_move_generation(benchmark::State& state) {
  Board board = Board::from_fen("r2q1rk1/2p2ppp/p7/1pbQp3/3n4/PB1P3P/1PP2PP1/RNB1R1K1 b - - 0 0");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_middlegame_move_generation);

static void board_endgame_move_generation(benchmark::State& state) {
  Board board = Board::from_fen("1r4k1/2p2ppp/p7/1p1Bp3/6P1/P1NPB2P/1P2K3/8 b - - 0 0");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_endgame_move_generation);

static void board_single_check_move_generation(benchmark::State& state) {
  Board board = Board::from_fen("rnbqk1nr/pppppppp/1b3Q2/8/N7/1P1PK3/PBP3PP/RN3B1R w kq - 0 0");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_single_check_move_generation);

static void board_double_check_move_generation(benchmark::State& state) {
  Board board = Board::from_fen("rnb1k1nr/pppppppp/1b2qQ2/8/N7/1P1PK3/PBP3PP/RN3B1R w kq - 0 0");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_double_check_move_generation);

static void board_quiescence_move_generation(benchmark::State& state) {
  Board board = Board::from_fen("r1b1k2r/2p2pb1/5n2/1qnpp2B/RP1PP1Q1/P6N/5Ppp/4K3 w kq - 0 0");
  for (auto _ : state) {
    MoveContainer moves = board.generate_quiescence_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_quiescence_move_generation);

static void board_quiescence_move_and_checks_generation(benchmark::State& state) {
  Board board = Board::from_fen("r1b1k2r/2p2pb1/3K1n2/1qnpp2B/RP1PP1Q1/P6N/5Ppp/8 b kq - 0 1 0 0");
  for (auto _ : state) {
    MoveContainer moves = board.generate_quiescence_moves_and_checks();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(board_quiescence_move_and_checks_generation);

static void board_has_moves(benchmark::State& state) {
  Board board = Board::from_fen("r1b1k2r/2p2pb1/3K1n2/1qnpp2B/RP1PP1Q1/P6N/5Ppp/8 b kq - 0 1 0 0");
  for (auto _ : state) {
    bool has_moves = board.has_moves();
    benchmark::DoNotOptimize(has_moves);
  }
}
BENCHMARK(board_has_moves);