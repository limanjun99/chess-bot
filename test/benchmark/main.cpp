#include <benchmark/benchmark.h>

#include <memory>

#include "chess_engine/ab_engine.h"
#include "chess_engine/engine.h"

static void alpha_beta_quiescence_search(benchmark::State& state) {
  Board board = Board::from_epd("r2qkb1r/pbpppppp/p4n2/3PP3/P1P5/1P3N2/RB2NPPP/Q4RK1 b kq -");
  std::unique_ptr<Engine> engine = std::make_unique<AlphaBetaEngine>(AlphaBetaEngine{5});
  for (auto _ : state) {
    Move move = (*engine).make_move(board);
    benchmark::DoNotOptimize(move);
  }
}

BENCHMARK(alpha_beta_quiescence_search);

BENCHMARK_MAIN();