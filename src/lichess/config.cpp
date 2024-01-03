#include "config.h"

#include <fstream>

#include "logger.h"

const std::string& Config::get_lichess_token() const { return lichess_token; }

const std::string& Config::get_lichess_bot_name() const { return lichess_bot_name; }

Config::Config() {
  std::ifstream env_file{".env"};
  std::string line;

  while (std::getline(env_file, line)) {
    if (line.empty()) continue;
    size_t equal_index = line.find("=");
    if (equal_index == std::string::npos) {
      Logger::warn() << "Extraneous line in .env file missing '=': " << line << "\n";
      continue;
    }

    std::string_view key = std::string_view{line}.substr(0, equal_index);
    std::string_view value = std::string_view{line}.substr(equal_index + 1);

    if (key == "LICHESS_TOKEN") {
      lichess_token = value;
    } else if (key == "LICHESS_BOT_NAME") {
      lichess_bot_name = value;
    } else {
      Logger::warn() << "Unknown key in .env file: " << line << "\n";
    }
  };

  if (lichess_token.empty()) {
    Logger::error() << "LICHESS_TOKEN not configured in .env\n";
    throw "LICHESS_TOKEN not configured in .env";
  }
  if (lichess_bot_name.empty()) {
    Logger::error() << "LICHESS_BOT_NAME not configured in .env\n";
    throw "LICHESS_BOT_NAME not configured in .env";
  }
}