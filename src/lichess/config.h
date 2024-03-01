#pragma once
#include <string>

class Config {
public:
  Config();

  const std::string& get_lichess_token() const;

  const std::string& get_lichess_bot_name() const;

  bool should_issue_challenges() const;

private:
  std::string lichess_token;     // API token with Bot permissions enabled.
  std::string lichess_bot_name;  // Username of the lichess bot.
  bool issue_challenges;         // Whether the bot should challenge other bots periodically.
};