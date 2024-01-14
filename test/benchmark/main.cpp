#include <benchmark/benchmark.h>

#include <memory>
#include <vector>

#include "chess/board.h"
#include "chess/move.h"
#include "chess_engine/engine.h"

static void initial_position_move_generation(benchmark::State& state) {
  Board board = Board::initial();
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}

BENCHMARK(initial_position_move_generation);

static void middlegame_move_generation(benchmark::State& state) {
  Board board = Board::from_epd("r2q1rk1/2p2ppp/p7/1pbQp3/3n4/PB1P3P/1PP2PP1/RNB1R1K1 b - -");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}

BENCHMARK(middlegame_move_generation);

static void endgame_move_generation(benchmark::State& state) {
  Board board = Board::from_epd("1r4k1/2p2ppp/p7/1p1Bp3/6P1/P1NPB2P/1P2K3/8 b - -");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}

BENCHMARK(endgame_move_generation);

static void single_check_move_generation(benchmark::State& state) {
  Board board = Board::from_epd("rnbqk1nr/pppppppp/1b3Q2/8/N7/1P1PK3/PBP3PP/RN3B1R w kq -");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(single_check_move_generation);

static void double_check_move_generation(benchmark::State& state) {
  Board board = Board::from_epd("rnb1k1nr/pppppppp/1b2qQ2/8/N7/1P1PK3/PBP3PP/RN3B1R w kq -");
  for (auto _ : state) {
    MoveContainer moves = board.generate_moves();
    benchmark::DoNotOptimize(moves);
  }
}
BENCHMARK(double_check_move_generation);

static void quiescence_move_generation(benchmark::State& state) {
  Board board = Board::from_epd("r1b1k2r/2p2pb1/5n2/1qnpp2B/RP1PP1Q1/P6N/5Ppp/4K3 w kq -");
  for (auto _ : state) {
    MoveContainer moves = board.generate_quiescence_moves();
    benchmark::DoNotOptimize(moves);
  }
}

BENCHMARK(quiescence_move_generation);

static void quiescence_move_and_checks_generation(benchmark::State& state) {
  Board board = Board::from_epd("r1b1k2r/2p2pb1/3K1n2/1qnpp2B/RP1PP1Q1/P6N/5Ppp/8 b kq - 0 1");
  for (auto _ : state) {
    MoveContainer moves = board.generate_quiescence_moves_and_checks();
    benchmark::DoNotOptimize(moves);
  }
}

BENCHMARK(quiescence_move_and_checks_generation);

static void board_has_moves(benchmark::State& state) {
  Board board = Board::from_epd("r1b1k2r/2p2pb1/3K1n2/1qnpp2B/RP1PP1Q1/P6N/5Ppp/8 b kq - 0 1");
  for (auto _ : state) {
    bool has_moves = board.has_moves();
    benchmark::DoNotOptimize(has_moves);
  }
}
BENCHMARK(board_has_moves);

void search(const Board& board, size_t current_depth) {
  if (current_depth <= 0) return;
  auto moves = board.generate_moves();
  for (size_t i = 0; i < moves.size(); i++) {
    search(board.apply_move(moves[i]), current_depth - 1);
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
  Board board = Board::from_epd("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_2);

static void perft_position_3(benchmark::State& state) {
  Board board = Board::from_epd("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
  for (auto _ : state) {
    search(board, 7);
  }
}
BENCHMARK(perft_position_3);

static void perft_position_4(benchmark::State& state) {
  Board board = Board::from_epd("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq ");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_4);

static void perft_position_5(benchmark::State& state) {
  Board board = Board::from_epd("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_5);

static void perft_position_6(benchmark::State& state) {
  Board board = Board::from_epd("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -");
  for (auto _ : state) {
    search(board, 5);
  }
}
BENCHMARK(perft_position_6);

BENCHMARK_MAIN();