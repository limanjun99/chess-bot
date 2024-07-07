#include "lichess.h"

#include <cpr/cpr.h>

#include <array>
#include <chrono>
#include <optional>
#include <random>
#include <sstream>
#include <thread>
#include <utility>

std::string api::online_bots(int bot_count) { return base + "/bot/online?nb=" + std::to_string(bot_count); }

std::string api::accept_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/accept";
}

std::string api::cancel_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/cancel";
}

std::string api::decline_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/decline";
}

std::string api::issue_challenge(const std::string& username) { return base + "/challenge/" + username; }

std::string api::stream_game(const std::string& game_id) { return base + "/bot/game/stream/" + game_id; }

std::string api::make_move(const std::string& game_id, const std::string& move) {
  return base + "/bot/game/" + game_id + "/move/" + move;
}

std::string api::profile() { return base + "/account"; }

Lichess::Lichess(const Config& config) : auth{config.get_lichess_token()} {}

std::vector<json> Lichess::get_online_bots(int limit) const {
  for (int i = 0; i < retry_limit; i++) {
    std::vector<json> bots;
    auto callback = [&](const json& bot) {
      if (!bot.is_null()) bots.push_back(bot);
      return true;
    };
    const auto response =
        cpr::Get(cpr::Url{api::online_bots(limit)}, auth, cpr::WriteCallback{NdjsonWriteCallback{callback}});
    if (rate_limit_check(response)) continue;
    return bots;
  }
  throw "Rate limited.";
}

bool Lichess::handle_challenge(const std::string& challenge_id, bool accept) const {
  for (int i = 0; i < retry_limit; i++) {
    const std::string url = accept ? api::accept_challenge(challenge_id) : api::decline_challenge(challenge_id);
    const auto response = cpr::Post(cpr::Url{std::move(url)}, auth);
    if (rate_limit_check(response)) continue;
    return response.status_code == 200;
  }
  throw "Rate limited.";
}

// https://lichess.org/api#tag/Challenges/operation/challengeCreate
std::optional<std::string> Lichess::issue_challenge(const std::string& username, bool rated, int limit,
                                                    int increment) const {
  for (int i = 0; i < retry_limit; i++) {
    const std::string url{api::issue_challenge(username)};
    const auto response = cpr::Post(cpr::Url{std::move(url)}, auth,
                                    cpr::Payload{{"rated", rated ? "true" : "false"},
                                                 {"clock.limit", std::to_string(limit)},
                                                 {"clock.increment", std::to_string(increment)},
                                                 {"color", "random"},
                                                 {"variant", "standard"}});
    if (rate_limit_check(response)) continue;
    if (response.status_code != 200) return std::nullopt;
    //! TODO: I think the API has changed to return what used to be in the "challenge" field directly.
    //! The lichess documentation is still unchanged though. Double check if the api changed.
    const auto challenge{json::parse(response.text)};
    if (challenge.contains("challenge")) return std::optional{challenge["challenge"]["id"]};
    else return std::optional{challenge["id"]};
  }
  throw "Rate limited.";
}

bool Lichess::cancel_challenge(const std::string& challenge_id) const {
  for (int i = 0; i < retry_limit; i++) {
    const auto response = cpr::Post(cpr::Url{api::cancel_challenge(challenge_id)}, auth);
    if (rate_limit_check(response)) continue;
    return response.status_code == 200;
  }
  throw "Rate limited.";
}

bool Lichess::send_move(const std::string& game_id, const std::string& move) const {
  for (int i = 0; i < retry_limit; i++) {
    const auto response = cpr::Post(cpr::Url{api::make_move(game_id, move)}, auth);
    if (rate_limit_check(response)) continue;
    return response.status_code == 200;
  }
  throw "Rate limited.";
}

json Lichess::get_my_profile() const {
  for (int i = 0; i < retry_limit; i++) {
    const auto response = cpr::Get(cpr::Url{api::profile()}, auth);
    if (rate_limit_check(response)) continue;
    return json::parse(response.text);
  }
  throw "Rate limited.";
}

bool Lichess::rate_limit_check(const cpr::Response& response) const {
  if (response.status_code == 429) {
    Logger::error() << "Rate limited by lichess api: " << response.text << "\n";
    Logger::flush();
    std::this_thread::sleep_for(std::chrono::seconds{90});
    return true;
  }
  return false;
}