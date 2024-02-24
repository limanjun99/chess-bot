#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <random>

#include "lichess.h"

using json = nlohmann::json;

class Bot {
public:
  Bot(const Config& config, const Lichess& lichess);

  // Start listening and responding to incoming events from Lichess API.
  void listen();

private:
  enum class State {
    Idle,
    InGame,
    IssueChallenge,  // We have exactly one issued challenge that is still pending.
  };

  const Config& config;
  const Lichess& lichess;
  std::chrono::time_point<std::chrono::steady_clock> state_from;
  State state;                      // Current state of this handler. Used to decide how to handle incoming events.
  std::mt19937 gen;                 // Used for any rng within this class (e.g. challenging a random online bot).
  std::string issued_challenge_id;  // Challenge id of the last issued challenge.
  std::vector<std::string> online_bots;
  std::chrono::time_point<std::chrono::steady_clock> online_bots_from;

  void change_state(State new_state);

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

  // Returns true if the bot is ready to issue a new challenge to another bot.
  bool is_ready_to_challenge() const;

  // Returns true if the bot is ready to refresh its list of online bots.
  bool is_ready_to_refresh_bots() const;

  // Issues a rated challenge to an online bot. This is called periodically.
  void issue_challenge();

  // Get a fresh list of online bots through the Lichess API. This is called periodically.
  void refresh_online_bots();

  friend class Lichess;
};