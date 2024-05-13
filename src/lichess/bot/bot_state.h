#pragma once

#include <memory>
#include <optional>

#include "../config.h"
#include "../lichess.h"

// The bot's behavior differs based on which state it is in.
class BotState {
public:
  BotState(const Config& config, const Lichess& lichess);

  // Refer to Lichess API for the shape of `challenge` object.
  virtual std::optional<std::unique_ptr<BotState>> handle_challenge(const json& challenge);

  // Refer to Lichess API for the shape of `challenge` object.
  virtual std::optional<std::unique_ptr<BotState>> handle_challenge_cancelled(const json& challenge);

  // Refer to Lichess API for the shape of `challenge` object.
  virtual std::optional<std::unique_ptr<BotState>> handle_challenge_declined(const json& challenge);

  // Refer to Lichess API for the shape of `game` object.
  virtual std::optional<std::unique_ptr<BotState>> handle_game_start(const json& game);

  // Refer to Lichess API for the shape of `game` object.
  virtual std::optional<std::unique_ptr<BotState>> handle_game_finish(const json& game);

  // When we open a connection to Lichess's event stream, it periodically sends an empty line.
  // This is used as a heartbeat for us to run periodic jobs.
  virtual std::optional<std::unique_ptr<BotState>> handle_null_event();

  virtual ~BotState() = default;

protected:
  const Config& config;
  const Lichess& lichess;

  // Check if a challenge is issued to us, and its conditions are acceptable.
  bool is_acceptable_challenge(const json& challenge);

  void log_declined_challenge(const json& challenge);

  void log_accepted_challenge(const json& challenge);

  void log_game_finish(const json& game);
};