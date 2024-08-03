#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>

#include "chess_engine/search.h"

struct Statistic {
  std::string id;
  bool solved;
  int64_t normal_node_count;
  int64_t quiescence_node_count;
  int64_t total_node_count;
  int32_t search_depth;
  std::chrono::milliseconds search_time;

  bool operator==(const Statistic& other) const;

  std::string to_string() const;

  // Parse a csv file of statistics and returns the statistics.
  static std::unordered_map<std::string, Statistic> parse_csv_file(const std::filesystem::path& file_path);

  // Writes the statistics to a csv file.
  static void write_csv_file(std::span<const Statistic> statistics, const std::filesystem::path& file_path);

  // Constructs a statistic for an unsolved puzzle.
  static Statistic make_unsolved(std::string id, const engine::Search::DebugInfo& debug);

  // Constructs a statistic for a solved puzzle.
  static Statistic make_solved(std::string id, const engine::Search::DebugInfo& debug);
};