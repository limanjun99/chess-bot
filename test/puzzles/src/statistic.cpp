#include "statistic.h"

#include <algorithm>
#include <format>
#include <string>

#include "file.h"

bool Statistic::operator==(const Statistic& other) const { return id == other.id; }

std::string Statistic::to_string() const {
  return std::format("{} - {:>6}k,{:>6}k,{:>6}k, depth {:>2}, time {:>6}", solved ? "✅" : "❌",
                     normal_node_count / 1000, quiescence_node_count / 1000, total_node_count / 1000, search_depth,
                     search_time);
}

namespace {

constexpr auto csv_header = std::string_view{"PuzzleId,Solved,NormalNodeCount,QuiescenceNodeCount,SearchDepth,TimeMS"};
constexpr auto csv_column_count = std::ranges::count(csv_header, ',') + 1;

Statistic parse_statistic(std::string_view line, const std::filesystem::path& file_path) {
  auto values = file::csv::split_line(line, csv_column_count, file_path);

  const auto id = values[0];
  const auto solved = static_cast<bool>(file::csv::parse_non_negative_integer<int32_t>(values[1]));
  const auto normal_node_count = file::csv::parse_non_negative_integer<int64_t>(values[2]);
  const auto quiescence_node_count = file::csv::parse_non_negative_integer<int64_t>(values[3]);
  const auto total_node_count = normal_node_count + quiescence_node_count;
  const auto search_depth = file::csv::parse_non_negative_integer<int32_t>(values[4]);
  const auto search_time = std::chrono::milliseconds{file::csv::parse_non_negative_integer<int64_t>(values[5])};

  return Statistic{
      .id = std::string{id},
      .solved = solved,
      .normal_node_count = normal_node_count,
      .quiescence_node_count = quiescence_node_count,
      .total_node_count = total_node_count,
      .search_depth = search_depth,
      .search_time = search_time,
  };
}

}  // namespace

std::unordered_map<std::string, Statistic> Statistic::parse_csv_file(const std::filesystem::path& file_path) {
  auto file = file::open_read(file_path);
  auto line = std::string{};

  std::getline(file, line);
  file::csv::check_header(line, csv_header, file_path);

  auto statistics = std::unordered_map<std::string, Statistic>{};
  while (std::getline(file, line)) {
    auto statistic = parse_statistic(line, file_path);
    auto id = statistic.id;
    statistics.insert({std::move(id), std::move(statistic)});
  }

  return statistics;
}

void Statistic::write_csv_file(std::span<const Statistic> statistics, const std::filesystem::path& file_path) {
  auto file = file::open_write(file_path);
  file << csv_header << '\n';
  for (const auto& statistic : statistics) {
    file << statistic.id << ',' << static_cast<int32_t>(statistic.solved) << ',' << statistic.normal_node_count << ','
         << statistic.quiescence_node_count << ',' << statistic.search_depth << ',' << statistic.search_time.count()
         << '\n';
  }
}

Statistic Statistic::make_unsolved(std::string id, const engine::Search::DebugInfo& debug) {
  return Statistic{.id = std::move(id),
                   .solved = false,
                   .normal_node_count = debug.normal_node_count,
                   .quiescence_node_count = debug.quiescence_node_count,
                   .total_node_count = debug.normal_node_count + debug.quiescence_node_count,
                   .search_depth = debug.search_depth,
                   .search_time = debug.time_spent};
}

Statistic Statistic::make_solved(std::string id, const engine::Search::DebugInfo& debug) {
  return Statistic{.id = std::move(id),
                   .solved = true,
                   .normal_node_count = debug.normal_node_count,
                   .quiescence_node_count = debug.quiescence_node_count,
                   .total_node_count = debug.normal_node_count + debug.quiescence_node_count,
                   .search_depth = debug.search_depth,
                   .search_time = debug.time_spent};
}