#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

#include "logger.h"

class Config {
public:
  static Config from_stream(std::istream& config_stream);

  const std::string& get_lichess_token() const;

  const std::string& get_lichess_bot_name() const;

  std::optional<std::filesystem::path> get_log_path() const;

  Logger::Level get_log_level() const;

  bool should_issue_challenges() const;

  std::chrono::minutes get_challenge_interval() const;

private:
  std::string lichess_token;                // API token with Bot permissions enabled.
  std::string lichess_bot_name;             // Username of the lichess bot.
  bool issue_challenges;                    // Whether the bot should challenge other bots periodically.
  std::chrono::minutes challenge_interval;  // If issue_challenges is true, then the bot should issue challenges after
                                            // idling for this interval.
  std::optional<std::filesystem::path> log_path;  // Path to write logs to. Should write to stdout if not provided.
  Logger::Level log_level;                        // What types of messages should be logged.

  explicit Config(std::string lichess_token, std::string lichess_bot_name, bool issue_challenges,
                  std::chrono::minutes challenge_interval, std::optional<std::filesystem::path> log_path,
                  Logger::Level log_level);
};