#include <benchmark/benchmark.h>

#include "chess_engine/naive_engine.h"

static void naive_engine_quiescence_search(benchmark::State& state) {
  Board board = Board::from_epd("r2qkb1r/pbpppppp/p4n2/3PP3/P1P5/1P3N2/RB2NPPP/Q4RK1 b kq -");
  NaiveEngine engine{0};
  for (auto _ : state) {
    Move move = engine.make_move(board);
    benchmark::DoNotOptimize(move);
  }
}

BENCHMARK(naive_engine_quiescence_search);

BENCHMARK_MAIN();