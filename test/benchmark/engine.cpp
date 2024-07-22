#include "chess_engine/engine.h"

#include <benchmark/benchmark.h>

#include <memory>
#include <vector>

#include "chess/board.h"
#include "chess_engine/engine.h"
#include "chess_engine/uci.h"

using namespace chess;

static void engine_initial_position_search(benchmark::State& state) {
  Board board = Board::initial();
  Engine engine{board};
  for (auto _ : state) {
    auto move_info = engine.search_sync(engine::uci::SearchConfig::from_depth(13));
    benchmark::DoNotOptimize(move_info);
  }
}
BENCHMARK(engine_initial_position_search);

static void engine_position_1_search(benchmark::State& state) {
  Board board = Board::from_fen("2r2r2/1Q2k1p1/p2p2q1/7p/P3Np2/7P/6P1/5R1K b - - 0 0");
  Engine engine{board};
  for (auto _ : state) {
    auto move_info = engine.search_sync(engine::uci::SearchConfig::from_depth(13));
    benchmark::DoNotOptimize(move_info);
  }
}
BENCHMARK(engine_position_1_search);