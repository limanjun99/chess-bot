#include "config.h"

#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

#include "logger.h"

const std::string& Config::get_lichess_token() const { return lichess_token; }

const std::string& Config::get_lichess_bot_name() const { return lichess_bot_name; }

const std::filesystem::path& Config::get_log_path() const { return log_path; }

Logger::Level Config::get_log_level() const { return log_level; }

bool Config::should_issue_challenges() const { return issue_challenges; }

namespace {

// Parses a line in the configuration stream. Returns pair<key, value>.
// Throws a `runtime_error` if the line is invalid.
std::pair<std::string_view, std::string_view> parse_line(std::string_view line) {
  const auto delimiter_index = line.find('=');
  if (delimiter_index == line.npos) {
    const auto error_message = std::format("Line in configuration file missing '=': {}", line);
    throw std::runtime_error{error_message};
  }

  const auto key = line.substr(0, delimiter_index);
  const auto value = line.substr(delimiter_index + 1);
  return {key, value};
}

}  // namespace

Config Config::from_stream(std::istream& config_stream) {
  auto lichess_token = std::optional<std::string>{};
  auto lichess_bot_name = std::optional<std::string>{};
  auto issue_challenges = std::optional<bool>{};
  auto log_path = std::optional<std::filesystem::path>{};
  auto log_level = std::optional<Logger::Level>{};

  auto line = std::string{};
  while (std::getline(config_stream, line)) {
    if (line.empty()) continue;
    const auto [key, value] = parse_line(line);

    if (key == "LICHESS_TOKEN") {
      lichess_token = value;
    } else if (key == "LICHESS_BOT_NAME") {
      lichess_bot_name = value;
    } else if (key == "ISSUE_CHALLENGES") {
      if (value == "TRUE") issue_challenges = true;
      else if (value == "FALSE") issue_challenges = false;
      else throw std::runtime_error{"Invalid configuration for ISSUE_CHALLENGES: Must be TRUE or FALSE."};
    } else if (key == "LOG_LEVEL") {
      log_level = Logger::parse_level(value);
    } else if (key == "LOG_PATH") {
      log_path = value;
    } else {
      const auto error_message = std::format("Unknown key in configuration file: {}", line);
      throw std::runtime_error{error_message};
    }
  };

  // Check for incomplete configuration.
  auto missing_keys = std::vector<std::string_view>{};
  if (!lichess_token) missing_keys.push_back("LICHESS_TOKEN");
  if (!lichess_bot_name) missing_keys.push_back("LICHESS_BOT_NAME");
  if (!issue_challenges) missing_keys.push_back("ISSUE_CHALLENGES");
  if (!log_level) missing_keys.push_back("LOG_LEVEL");
  if (!log_path) missing_keys.push_back("LOG_PATH");
  if (!missing_keys.empty()) {
    auto missing_keys_string = std::string{};
    missing_keys_string += missing_keys[0];
    for (const auto& missing_key : missing_keys | std::views::drop(1)) {
      missing_keys_string += ",";
      missing_keys_string += missing_key;
    }
    const auto error_message = std::format("Missing keys in configuration file: {}", missing_keys_string);
    throw std::runtime_error{error_message};
  }

  return Config{*std::move(lichess_token), *std::move(lichess_bot_name), *std::move(issue_challenges),
                *std::move(log_path), *std::move(log_level)};
}

Config::Config(std::string lichess_token, std::string lichess_bot_name, bool issue_challenges,
               std::filesystem::path log_path, Logger::Level log_level)
    : lichess_token{std::move(lichess_token)},
      lichess_bot_name{std::move(lichess_bot_name)},
      issue_challenges{issue_challenges},
      log_path{std::move(log_path)},
      log_level{log_level} {}