#include "chess_engine/engine.h"

#include <benchmark/benchmark.h>

#include <memory>
#include <vector>

#include "chess/board.h"
#include "chess/move.h"
#include "chess_engine/engine.h"

static void engine_initial_position_search(benchmark::State& state) {
  Engine engine{};
  Board board = Board::initial();
  for (auto _ : state) {
    auto move_info = engine.choose_move(board, 13);
    benchmark::DoNotOptimize(move_info);
  }
}
BENCHMARK(engine_initial_position_search);

static void engine_position_1_search(benchmark::State& state) {
  Engine engine{};
  Board board = Board::from_fen("2r2r2/1Q2k1p1/p2p2q1/7p/P3Np2/7P/6P1/5R1K b - - 0 0");
  for (auto _ : state) {
    auto move_info = engine.choose_move(board, 13);
    benchmark::DoNotOptimize(move_info);
  }
}
BENCHMARK(engine_position_1_search);