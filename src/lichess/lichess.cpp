#include "lichess.h"

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>
#include <string>

#include "logger.h"

using json = nlohmann::json;

namespace api {
std::string base{"https://lichess.org/api"};
std::string stream_incoming_events{base + "/stream/event"};
std::string accept_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/accept";
}
std::string decline_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/decline";
}
};  // namespace api

Lichess::Lichess(const Config& config) : config{config} {}

void Lichess::listen() {
  auto callback = [this](std::string data, intptr_t) { return handle_incoming_event(std::move(data)); };
  cpr::Get(cpr::Url{api::stream_incoming_events}, cpr::Bearer{config.get_lichess_token()},
           cpr::WriteCallback{callback});
}

bool Lichess::handle_challenge(const std::string& challenge_id, bool accept) {
  std::string url = accept ? api::accept_challenge(challenge_id) : api::decline_challenge(challenge_id);
  cpr::Response res = cpr::Post(cpr::Url{std::move(url)}, cpr::Bearer{config.get_lichess_token()});
  return res.status_code == 200;
}

bool Lichess::handle_incoming_event(std::string data) {
  while (!data.empty() && data.back() == '\n') data.pop_back();
  if (data.empty()) return true;
  json event = json::parse(data);

  if (event["type"] == "challenge") {
    auto challenge = event["challenge"];
    // Ignore all requests sent out by the bot.
    if (challenge["challenger"]["id"] == config.get_lichess_bot_name()) return true;
    // Decline all rated challenges.
    if (challenge["rated"]) {
      handle_challenge(challenge["id"], false);
      Logger::info() << "Declined challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
      return true;
    }
    // Accept all other challenges.
    handle_challenge(challenge["id"], true);
    Logger::info() << "Accepted challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
  }
  return true;
}
