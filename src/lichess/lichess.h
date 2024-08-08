#pragma once

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>
#include <optional>
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

std::string cancel_challenge(const std::string& challenge_id);

std::string decline_challenge(const std::string& challenge_id);

std::string issue_challenge(const std::string& username);

std::string stream_game(const std::string& game_id);

std::string make_move(const std::string& game_id, const std::string& move);

std::string profile();
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

template <typename T>
concept is_game_handler = requires(T v) {
  { v.handle_game_initialization(std::declval<json>()) } -> std::convertible_to<bool>;
  { v.handle_game_update(std::declval<json>()) } -> std::convertible_to<bool>;
  { v.handle_game_end(std::declval<json>()) };
};

// An interface for communicating with the Lichess API.
class Lichess {
public:
  Lichess(const Config& config);

  // Returns a list of (up to `limit`) bots that are currently online.
  std::vector<json> get_online_bots(int limit) const;

  // Either accept or decline the challenge with given `challenge_id`.
  // Returns false if challenge was not found.
  bool handle_challenge(const std::string& challenge_id, bool accept) const;

  // Handle the ongoing game with given `game_id`.
  // void handle_game(const std::string& game_id, std);

  // Listen for incoming events and handle them using the `handler`.
  template <is_event_handler T>
  void handle_incoming_events(T& handler) const;

  // Listen for events for the game with `game_id`, and handle them using `handler`.
  template <is_game_handler T>
  void handle_game(const std::string& game_id, T& handler) const;

  // Issues a challenge to another account of the given `username`.
  // `rated` - Whether the challenge is rated.
  // `limit` - Initial clock time (in seconds).
  // `increment` - Clock increment per move (in seconds).
  // Returns the id of the created challenge, or nullopt if creation failed.
  std::optional<std::string> issue_challenge(const std::string& username, bool rated, int limit, int increment) const;

  // Cancels a challenge with the given `challenge_id`.
  // Returns true if the challenge was cancelled successfully.
  bool cancel_challenge(const std::string& challenge_id) const;

  // Make a move on the game with `gameId`. `move` is in UCI format.
  // Returns true if the move was made successfully.
  bool send_move(const std::string& game_id, const std::string& move) const;

  json get_my_profile() const;

private:
  cpr::Bearer auth;
  static constexpr int retry_limit = 4;  // If we get rate limited this number of times, then terminate the program.

  // If we are being rate limited, sleep for 90 seconds and return true.
  bool rate_limit_check(const cpr::Response& response) const;
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
      Logger::get().format_error("Unhandled event: {}", event.dump());
      throw "Unhandled event";
    }
    return true;
  };
  cpr::Get(cpr::Url{api::stream_incoming_events}, auth, cpr::WriteCallback{NdjsonWriteCallback{callback}});
}

template <is_game_handler T>
inline void Lichess::handle_game(const std::string& game_id, T& handler) const {
  auto callback = [&](const json& event) {
    // Ignore null / chat / opponent gone events.
    //! TODO: Maybe check if opponent is gone for too long and claim win?
    if (event.is_null() || event["type"] == "chatLine" || event["type"] == "opponentGone") {
      return true;
    }

    const auto& state = event["type"] == "gameFull" ? event["state"] : event;
    if (state["status"] != "created" && state["status"] != "started") {
      // Game has ended (e.g. win / lose / draw / aborted).
      handler.handle_game_end(state);
      return false;
    }

    if (event["type"] == "gameFull") {
      return handler.handle_game_initialization(event);
    } else if (event["type"] == "gameState") {
      return handler.handle_game_update(event);
    }

    Logger::get().format_error("Game received invalid event: {}", event.dump());
    throw "Game received invalid event";
  };
  const std::string url = api::stream_game(game_id);
  cpr::Get(cpr::Url{std::move(url)}, auth, cpr::WriteCallback{NdjsonWriteCallback{callback}});
}
