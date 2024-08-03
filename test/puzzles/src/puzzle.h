#pragma once

#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>

#include "chess/board.h"
#include "chess/move.h"

struct Puzzle {
  std::string id;
  chess::Board board;
  chess::Move solution;
  int32_t rating;

  bool operator==(const Puzzle& other) const;

  // Parse a csv file of puzzles and returns the puzzles.
  static std::unordered_map<std::string, Puzzle> parse_csv_file(const std::filesystem::path& file_path);

  // Writes the puzzles to a csv file.
  static void write_csv_file(std::span<const Puzzle> puzzles, const std::filesystem::path& file_path);
};
