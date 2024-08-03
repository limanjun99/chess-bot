#pragma once

#include <filesystem>
#include <optional>

struct Config {
  std::filesystem::path folder_path;
  std::optional<std::filesystem::path> lichess_open_db_path;

  // Constructs a configuration object from command line arguments.
  // Throws a `runtime_error` if the arguments are invalid.
  static Config from_cli(int argc, char* argv[]);
};