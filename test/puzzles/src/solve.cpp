#include "solve.h"

#include <chrono>
#include <cstdint>
#include <print>

#include "chess_engine/engine.h"

namespace {

constexpr auto search_time_limit = std::chrono::milliseconds{1000};
constexpr auto search_depth_limit = 48;

}  // namespace

Statistic solve::puzzle(const Puzzle& puzzle) {
  auto engine = Engine{};
  for (auto depth = 6; depth <= search_depth_limit; depth += 2) {
    engine.reset();
    engine.set_position(puzzle.board);
    const auto search_config = engine::uci::SearchConfig{}.set_depth(depth).set_movetime(search_time_limit);
    const auto [move, debug] = engine.search_sync(search_config);

    if (debug.timed_out) {
      // Timed out means the engine did not reach our target depth, so this run is invalid.
      return Statistic::make_unsolved(puzzle.id, debug);
    }

    if (move == puzzle.solution) {
      return Statistic::make_solved(puzzle.id, debug);
    }

    if (depth + 2 > search_depth_limit) {
      // Hit search limit without finding solution.
      return Statistic::make_unsolved(puzzle.id, debug);
    }
  }

  throw std::runtime_error("Unreachable");
}

void solve::compare(const Statistic& new_statistic, const Statistic& old_statistics) {
  std::print("{}: {}  |  {}", new_statistic.id, old_statistics.to_string(), new_statistic.to_string());

  if (!old_statistics.solved && new_statistic.solved) {
    std::print("   [🎉 new puzzle solved!]");
  } else if (old_statistics.solved && !new_statistic.solved) {
    std::print("   [⚠️⚠️⚠️ REGRESSION]");
  } else if (old_statistics.solved && new_statistic.solved) {
    const auto node_count_delta = new_statistic.total_node_count - old_statistics.total_node_count;
    const auto node_count_percentage = static_cast<double>(node_count_delta) / old_statistics.total_node_count;
    const auto time_delta = new_statistic.search_time - old_statistics.search_time;
    const auto time_percentage = static_cast<double>(time_delta.count()) / old_statistics.search_time.count();
    std::print("   [Nodes: {:>6.2f}%, Time: {:>6.2f}%]", node_count_percentage * 100, time_percentage * 100);
  }

  std::println("");
}

namespace {

struct TotalStatistic {
  int32_t solved;
  int64_t nodes;
  std::chrono::milliseconds time;

  static TotalStatistic from_statistics(std::span<const Statistic> statistics);
};

TotalStatistic TotalStatistic::from_statistics(std::span<const Statistic> statistics) {
  auto total = TotalStatistic{};
  for (const auto& statistic : statistics) {
    if (!statistic.solved) continue;
    total.solved += 1;
    total.nodes += statistic.total_node_count;
    total.time += statistic.search_time;
  }
  return total;
}

}  // namespace

void solve::compare_all(std::span<const Statistic> new_statistics, std::span<const Statistic> old_statistics) {
  const auto new_total = TotalStatistic::from_statistics(new_statistics);
  const auto old_total = TotalStatistic::from_statistics(old_statistics);

  const auto solved_delta = new_total.solved - old_total.solved;
  const auto nodes_delta = new_total.nodes - old_total.nodes;
  const auto nodes_percentage = static_cast<double>(nodes_delta) / old_total.nodes;
  const auto time_delta = new_total.time - old_total.time;
  const auto time_percentage = static_cast<double>(time_delta.count()) / old_total.time.count();

  std::println("");
  std::println("     {:>4} {:>11} {:>9}", "✅", "Nodes", "Time");
  std::println("Old: {:>4} {:>11} {:>9}", old_total.solved, old_total.nodes, old_total.time);
  std::println("New: {:>4} {:>11} {:>9}", new_total.solved, new_total.nodes, new_total.time);
  std::println("     {:>4} {:>10.2f}% {:>8.2f}%", solved_delta, nodes_percentage * 100, time_percentage * 100);
}