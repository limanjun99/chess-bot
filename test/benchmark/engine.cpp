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
    auto move_info = engine.choose_move(board, 9);
    benchmark::DoNotOptimize(move_info);
  }
}

BENCHMARK(engine_initial_position_search);
