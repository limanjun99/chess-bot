#include "lichess.h"

#include <cpr/cpr.h>

#include <array>
#include <nlohmann/json.hpp>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <utility>

#include "chess_engine/engine.h"
#include "game_handler.h"
#include "logger.h"

using json = nlohmann::json;

std::string api::online_bots(int bot_count) { return base + "/bot/online?nb=" + std::to_string(bot_count); }

std::string api::accept_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/accept";
}

std::string api::decline_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/decline";
}

std::string api::issue_challenge(const std::string& username) { return base + "/challenge/" + username; }

std::string api::stream_game(const std::string& game_id) { return base + "/bot/game/stream/" + game_id; }

std::string api::make_move(const std::string& game_id, const std::string& move) {
  return base + "/bot/game/" + game_id + "/move/" + move;
}

Lichess::Lichess(const Config& config) : auth{config.get_lichess_token()} {}

std::vector<std::string> Lichess::get_online_bots(int limit) const {
  std::vector<std::string> usernames;
  auto callback = [&](const json& bot) {
    if (!bot.is_null()) usernames.push_back(bot["username"]);
    return true;
  };
  cpr::Get(cpr::Url{api::online_bots(limit)}, auth, cpr::WriteCallback{NdjsonWriteCallback{callback}});
  return usernames;
}

bool Lichess::handle_challenge(const std::string& challenge_id, bool accept) const {
  const std::string url = accept ? api::accept_challenge(challenge_id) : api::decline_challenge(challenge_id);
  const auto response = cpr::Post(cpr::Url{std::move(url)}, auth);
  return response.status_code == 200;
}

// https://lichess.org/api#tag/Challenges/operation/challengeCreate
bool Lichess::issue_challenge(const std::string& username, bool rated, int limit, int increment) const {
  const std::string url{api::issue_challenge(username)};
  const auto response = cpr::Post(cpr::Url{std::move(url)}, auth,
                                  cpr::Payload{{"rated", rated ? "true" : "false"},
                                               {"clock.limit", std::to_string(limit)},
                                               {"clock.increment", std::to_string(increment)},
                                               {"color", "random"},
                                               {"variant", "standard"}});
  return response.status_code == 200;
}
