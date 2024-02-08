#pragma once

#include <chrono>
#include <string>

#include "config.h"

namespace lichess_api {
static std::string base{"https://lichess.org/api"};
static std::string stream_incoming_events{base + "/stream/event"};
std::string online_bots(int bot_count);
std::string accept_challenge(const std::string& challenge_id);
std::string decline_challenge(const std::string& challenge_id);
std::string issue_challenge(const std::string& username);
std::string stream_game(const std::string& game_id);
std::string make_move(const std::string& game_id, const std::string& move);
};  // namespace lichess_api

class Lichess {
public:
  Lichess(const Config& config);

  // Start listening and responding to incoming events from Lichess API.
  void listen();

private:
  const Config& config;
  std::chrono::time_point<std::chrono::system_clock> last_challenge_time{std::chrono::system_clock::now()};

  // Either accept or decline the challenge with given `challenge_id`.
  // Returns false if challenge was not found.
  bool handle_challenge(const std::string& challenge_id, bool accept);

  // Handle the ongoing game with given `game_id`.
  void handle_game(const std::string& game_id);

  // Handle an incoming event. Returns true if handled successfully.
  bool handle_incoming_event(std::string data);

  // Issues a rated challenge to an online bot. This is called periodically.
  void issue_challenge();
};
