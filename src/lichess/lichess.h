#pragma once

#include <cpr/cpr.h>

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "config.h"
#include "logger.h"
#include "ndjson.h"

using json = nlohmann::json;

// Url for various Lichess API endpoints.
namespace api {
const std::string base{"https://lichess.org/api"};

const std::string stream_incoming_events{base + "/stream/event"};

std::string online_bots(int bot_count);

std::string accept_challenge(const std::string& challenge_id);

std::string decline_challenge(const std::string& challenge_id);

std::string issue_challenge(const std::string& username);

std::string stream_game(const std::string& game_id);

std::string make_move(const std::string& game_id, const std::string& move);
};  // namespace api

template <typename T>
concept is_event_handler = requires(T v) {
  { v.handle_challenge(std::declval<json>()) };
  { v.handle_challenge_cancelled(std::declval<json>()) };
  { v.handle_challenge_declined(std::declval<json>()) };
  { v.handle_game_start(std::declval<json>()) };
  { v.handle_game_finish(std::declval<json>()) };
  { v.handle_null_event() };
};

// An interface for communicating with the Lichess API.
class Lichess {
public:
  Lichess(const Config& config);

  // Returns a list of (up to `limit`) bots that are currently online.
  std::vector<std::string> get_online_bots(int limit) const;

  // Either accept or decline the challenge with given `challenge_id`.
  // Returns false if challenge was not found.
  bool handle_challenge(const std::string& challenge_id, bool accept) const;

  // Handle the ongoing game with given `game_id`.
  // void handle_game(const std::string& game_id, std);

  // Listen for incoming events and handle them using the `handler`.
  template <is_event_handler T>
  void handle_incoming_events(T& handler) const;

  // Issues a challenge to another account of the given `username`.
  // `rated` - Whether the challenge is rated.
  // `limit` - Initial clock time (in seconds).
  // `increment` - Clock increment per move (in seconds).
  // Returns true if the challenge was created successfully.
  bool issue_challenge(const std::string& username, bool rated, int limit, int increment) const;

private:
  cpr::Bearer auth;
};

// =============== IMPLEMENTATIONS ===============

template <is_event_handler T>
inline void Lichess::handle_incoming_events(T& handler) const {
  auto callback = [&](const json& event) {
    if (event.is_null()) {
      handler.handle_null_event();
    } else if (event["type"] == "challenge") {
      handler.handle_challenge(event["challenge"]);
    } else if (event["type"] == "challengeCanceled") {
      handler.handle_challenge_cancelled(event["challenge"]);
    } else if (event["type"] == "challengeDeclined") {
      handler.handle_challenge_declined(event["challenge"]);
    } else if (event["type"] == "gameStart") {
      handler.handle_game_start(event["game"]);
    } else if (event["type"] == "gameFinish") {
      handler.handle_game_finish(event["game"]);
    } else {
      Logger::error() << "Unhandled event: " << event << "\n";
      Logger::flush();
      throw "Unhandled event";
    }
    return true;
  };
  cpr::Get(cpr::Url{api::stream_incoming_events}, auth, cpr::WriteCallback{NdjsonWriteCallback{callback}});
}