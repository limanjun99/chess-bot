#include "puzzle.h"

#include <algorithm>
#include <string>

#include "chess/uci.h"
#include "file.h"

bool Puzzle::operator==(const Puzzle& other) const { return id == other.id; }

namespace {

constexpr auto csv_header = std::string_view{"PuzzleId,FEN,SolutionMove,Rating"};
constexpr auto csv_column_count = std::ranges::count(csv_header, ',') + 1;

Puzzle parse_puzzle(std::string_view line, const std::filesystem::path& file_path) {
  auto values = file::csv::split_line(line, csv_column_count, file_path);

  const auto id = values[0];
  auto board = chess::Board::from_fen(values[1]);
  auto solution = chess::uci::move(values[2], board);
  const auto rating = file::csv::parse_non_negative_integer<int32_t>(values[3]);

  return Puzzle{.id = std::string{id}, .board = std::move(board), .solution = std::move(solution), .rating = rating};
}

}  // namespace

std::unordered_map<std::string, Puzzle> Puzzle::parse_csv_file(const std::filesystem::path& file_path) {
  auto file = file::open_read(file_path);
  auto line = std::string{};

  std::getline(file, line);
  file::csv::check_header(line, csv_header, file_path);

  auto puzzles = std::unordered_map<std::string, Puzzle>{};
  while (std::getline(file, line)) {
    auto puzzle = parse_puzzle(line, file_path);
    auto id = puzzle.id;
    puzzles.insert({std::move(id), std::move(puzzle)});
  }

  return puzzles;
}

void Puzzle::write_csv_file(std::span<const Puzzle> puzzles, const std::filesystem::path& file_path) {
  auto file = file::open_write(file_path);
  file << csv_header << '\n';
  for (const auto& puzzle : puzzles) {
    file << puzzle.id << ',' << puzzle.board.to_fen() << ',' << puzzle.solution.to_uci() << ',' << puzzle.rating
         << '\n';
  }
}