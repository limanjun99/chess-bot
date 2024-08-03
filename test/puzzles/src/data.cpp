#include "data.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <random>
#include <ranges>
#include <string>

#include "chess/board.h"
#include "chess/uci.h"
#include "chess_engine/engine.h"
#include "file.h"
#include "statistic.h"

namespace {

constexpr auto puzzles_file_name = std::string_view{"puzzles.csv"};
constexpr auto statistics_file_name = std::string_view{"statistics.csv"};
constexpr auto updated_statistics_file_name = std::string_view{"new_statistics.csv"};

}  // namespace

namespace {

// Validates that every puzzle and statistic has a matching pairing, else throws a `runtime_error`.
void validate_data(const std::unordered_map<std::string, Puzzle>& puzzles,
                   const std::unordered_map<std::string, Statistic>& statistics) {
  for (const auto& [id, _] : puzzles) {
    if (statistics.contains(id)) continue;
    const auto error_message = std::format("Missing statistic for puzzle: {}", id);
    throw std::runtime_error{error_message};
  }

  for (const auto& [id, _] : statistics) {
    if (puzzles.contains(id)) continue;
    const auto error_message = std::format("Missing puzzle for statistic: {}", id);
    throw std::runtime_error{error_message};
  }
}

}  // namespace

std::vector<std::pair<Puzzle, Statistic>> data::get(const Config& config) {
  const auto puzzle_path = config.folder_path / puzzles_file_name;
  const auto statistic_path = config.folder_path / statistics_file_name;

  const auto puzzles = Puzzle::parse_csv_file(puzzle_path);
  const auto statistics = Statistic::parse_csv_file(statistic_path);

  validate_data(puzzles, statistics);

  auto data = std::vector<std::pair<Puzzle, Statistic>>{};
  data.reserve(puzzles.size());
  for (const auto& [id, puzzle] : puzzles) {
    const auto& statistic = statistics.at(id);
    data.emplace_back(puzzle, statistic);
  }

  std::ranges::sort(data, [](const auto& left, const auto& right) { return left.second.id < right.second.id; });

  return data;
}

namespace {

constexpr auto db_header =
    std::string_view{"PuzzleId,FEN,Moves,Rating,RatingDeviation,Popularity,NbPlays,Themes,GameUrl,OpeningTags"};
constexpr auto db_column_count = std::ranges::count(db_header, ',') + 1;

// Returns std::nullopt if the puzzle should be ignored.
std::optional<Puzzle> parse_db_line(std::string_view line, const std::filesystem::path& db_path) {
  auto values = file::csv::split_line(line, db_column_count, db_path);

  // Skip all mateIn1 puzzles, as their solution is not guaranteed to be unique.
  const auto& theme = values[7];
  if (theme.contains("mateIn1")) return std::nullopt;

  // Skip all puzzles with a rating of <2500, or high deviation of >100.
  const auto rating = file::csv::parse_non_negative_integer<int32_t>(values[3]);
  const auto rating_deviation = file::csv::parse_non_negative_integer<int32_t>(values[4]);
  if (rating < 2500 || rating_deviation > 100) return std::nullopt;

  const auto id = std::move(values[0]);
  auto board = chess::Board::from_fen(values[1]);

  // values[2] is a space-delimited string of moves in uci format.
  // The first move (next_move) should be applied to FEN to get the puzzle position.
  // The second move (solution_move) is the solution to the puzzle.
  const auto moves = values[2] | std::views::split(' ') | std::views::take(2) |
                     std::views::transform([](auto&& range) { return std::string_view{range}; }) |
                     std::ranges::to<std::vector>();
  const auto& next_move_uci = moves[0];
  const auto& solution_move_uci = moves[1];

  board = board.apply_move(chess::uci::move(next_move_uci, board));
  auto solution = chess::uci::move(solution_move_uci, board);

  return Puzzle{.id = std::string{id}, .board = std::move(board), .solution = std::move(solution), .rating = rating};
}

}  // namespace

void data::generate(const Config& config) {
  const auto& db_path = config.lichess_open_db_path.value();
  auto db_file = file::open_read(db_path);
  auto line = std::string{};

  std::getline(db_file, line);
  file::csv::check_header(line, db_header, db_path);

  // Parse puzzles.
  auto puzzles = std::vector<Puzzle>{};
  while (std::getline(db_file, line)) {
    if (auto puzzle = parse_db_line(line, db_path)) {
      puzzles.push_back(std::move(puzzle).value());
    }
  }

  // Choose random sample of puzzles.
  auto rng = std::mt19937{0};
  std::shuffle(puzzles.begin(), puzzles.end(), rng);
  constexpr auto sample_size = 100;
  auto chosen_puzzles = std::vector<Puzzle>{};
  for (const auto& puzzle : puzzles) {
    // Filter out puzzles that can easily be solved at depth 6.
    const auto& [move, _] = Engine{puzzle.board}.search_sync(engine::uci::SearchConfig::from_depth(6));
    if (move == puzzle.solution) continue;

    chosen_puzzles.push_back(puzzle);
    if (chosen_puzzles.size() >= sample_size) break;
  }
  std::ranges::sort(chosen_puzzles, [](const auto& left, const auto& right) { return left.id < right.id; });

  auto statistics = std::vector<Statistic>{};
  statistics.reserve(chosen_puzzles.size());
  for (const auto& puzzle : chosen_puzzles) {
    auto statistic = Statistic{.id = puzzle.id,
                               .solved = false,
                               .normal_node_count = 0,
                               .quiescence_node_count = 0,
                               .total_node_count = 0,
                               .search_depth = 0,
                               .search_time = std::chrono::milliseconds{0}};
    statistics.push_back(std::move(statistic));
  }

  // Write to files.
  Puzzle::write_csv_file(chosen_puzzles, config.folder_path / puzzles_file_name);
  Statistic::write_csv_file(statistics, config.folder_path / statistics_file_name);
}

void data::update_statistics(std::span<const Statistic> statistics, const Config& config) {
  Statistic::write_csv_file(statistics, config.folder_path / updated_statistics_file_name);
}