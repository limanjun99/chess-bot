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

std::string lichess_api::online_bots(int bot_count) { return base + "/bot/online?nb=" + std::to_string(bot_count); }
std::string lichess_api::accept_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/accept";
}
std::string lichess_api::decline_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/decline";
}
std::string lichess_api::issue_challenge(const std::string& username) { return base + "/challenge/" + username; }
std::string lichess_api::stream_game(const std::string& game_id) { return base + "/bot/game/stream/" + game_id; }
std::string lichess_api::make_move(const std::string& game_id, const std::string& move) {
  return base + "/bot/game/" + game_id + "/move/" + move;
}

Lichess::Lichess(const Config& config) : config{config} {}

void Lichess::listen() {
  auto callback = [this](std::string data, intptr_t) {
    if (data == "\n") {
      // Issue a new challenge if our last challenge was >45 seconds ago.
      if (std::chrono::system_clock::now() - last_challenge_time > std::chrono::seconds{45}) {
        last_challenge_time = std::chrono::system_clock::now();
        issue_challenge();
      }
      return true;
    } else {
      return handle_incoming_event(std::move(data));
    }
  };
  cpr::Get(cpr::Url{lichess_api::stream_incoming_events}, cpr::Bearer{config.get_lichess_token()},
           cpr::WriteCallback{callback});
}

bool Lichess::handle_challenge(const std::string& challenge_id, bool accept) {
  std::string url = accept ? lichess_api::accept_challenge(challenge_id) : lichess_api::decline_challenge(challenge_id);
  cpr::Response res = cpr::Post(cpr::Url{std::move(url)}, cpr::Bearer{config.get_lichess_token()});
  return res.status_code == 200;
}

void Lichess::handle_game(const std::string& game_id) {
  GameHandler game_handler{config, game_id};
  game_handler.listen();
}

bool Lichess::handle_incoming_event(std::string data) {
  while (!data.empty() && data.back() == '\n') data.pop_back();
  if (data.empty()) return true;
  json event = json::parse(data);

  if (event["type"] == "challenge") {
    auto challenge = event["challenge"];

    // Ignore all events that are not other players challenging us.
    if (challenge["status"] != "created" || challenge["challenger"]["id"] == config.get_lichess_bot_name()) return true;

    // Decline all classical / correspondance or non-standard chess challenges.
    if (challenge["speed"] == "classical" || challenge["speed"] == "correspondence" ||
        challenge["variant"]["key"] != "standard") {
      handle_challenge(challenge["id"], false);
      Logger::info() << "Declined challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
      Logger::flush();
      return true;
    }

    // Accept all other challenges.
    handle_challenge(challenge["id"], true);
    Logger::info() << "Accepted challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
    Logger::flush();
  } else if (event["type"] == "gameStart") {
    auto game = event["game"];
    handle_game(game["gameId"]);
  } else if (event["type"] == "challengeDeclined") {
    Logger::info() << "Challenge to " << event["challenge"]["destUser"]["name"] << " was declined because '"
                   << event["challenge"]["declineReason"] << "'\n";
    Logger::flush();
  }
  return true;
}

void Lichess::issue_challenge() {
  // List of initial time and increment (in seconds) to choose from.
  //! TODO: Add no-increment clocks once we handle network latency.
  constexpr std::array<std::pair<int, int>, 3> clocks = {{
      {120, 1},  // 2+1 Bullet
      {180, 2},  // 3+2 Blitz
      {300, 3},  // 5+3 Blitz
  }};

  const int clock_choice = rand() % clocks.size();
  const int clock_time = clocks[clock_choice].first;
  const int clock_increment = clocks[clock_choice].second;

  std::string online_bots_data;
  auto online_bots_callback = [&](std::string data, intptr_t) {
    online_bots_data += data;
    return true;
  };
  const cpr::Response res = cpr::Get(cpr::Url{lichess_api::online_bots(150)}, cpr::Bearer{config.get_lichess_token()},
                                     cpr::WriteCallback{online_bots_callback});
  std::vector<std::string> usernames;
  std::stringstream input{online_bots_data};
  std::string line;
  while (std::getline(input, line)) {
    if (line.empty()) continue;
    auto bot = json::parse(line);
    usernames.push_back(bot["username"]);
  }
  if (usernames.empty()) {
    Logger::warn() << "Did not manage to find an online bot to challenge.\n";
    Logger::flush();
    return;
  }

  int username_index = rand() % usernames.size();
  const std::string username = usernames[username_index];
  const std::string url{lichess_api::issue_challenge(username)};

  Logger::info() << "Issuing challenge to " << username << "\n";
  Logger::flush();
  cpr::Post(cpr::Url{std::move(url)}, cpr::Bearer{config.get_lichess_token()},
            cpr::Payload{{"rated", "false"},
                         {"clock.limit", std::to_string(clock_time)},
                         {"clock.increment", std::to_string(clock_increment)},
                         {"color", "random"},
                         {"variant", "standard"}});
}
