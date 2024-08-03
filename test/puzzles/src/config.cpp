#include "config.h"

constexpr auto help_message =
    "Usage (to test):     ./puzzles <path_to_puzzles_dataset_folder>\n"
    "Usage (to generate): ./puzzles <path_to_puzzles_dataset_folder> <path_to_lichess_open_db_csv_file>";

Config Config::from_cli(int argc, char* argv[]) {
  if (argc == 2) {
    return Config{.folder_path = std::filesystem::path{argv[1]}, .lichess_open_db_path = std::nullopt};
  }

  if (argc == 3) {
    return Config{
        .folder_path = std::filesystem::path{argv[1]},
        .lichess_open_db_path = std::filesystem::path{argv[2]},
    };
  }

  throw std::runtime_error{help_message};
}