#pragma once

#include <nlohmann/json.hpp>

#include "bot/bot_state.h"
#include "lichess.h"

using json = nlohmann::json;

class Bot {
public:
  Bot(const Config& config, const Lichess& lichess);

  // Start listening and responding to incoming events from Lichess API.
  void listen();

private:
  const Config& config;
  const Lichess& lichess;
  std::unique_ptr<BotState> state;  // Current state of this handler. Used to decide how to handle incoming events.

  void change_state(std::unique_ptr<BotState> new_state);
  void change_state(std::optional<std::unique_ptr<BotState>> new_state);

  // Refer to Lichess API for the shape of `challenge` object.
  void handle_challenge(const json& challenge);

  // Refer to Lichess API for the shape of `challenge` object.
  void handle_challenge_cancelled(const json& challenge);

  // Refer to Lichess API for the shape of `challenge` object.
  void handle_challenge_declined(const json& challenge);

  // Refer to Lichess API for the shape of `game` object.
  void handle_game_start(const json& game);

  // Refer to Lichess API for the shape of `game` object.
  void handle_game_finish(const json& game);

  // When we open a connection to Lichess's event stream, it periodically sends an empty line.
  // This is used as a heartbeat for us to run periodic jobs.
  void handle_null_event();

  friend class Lichess;
};