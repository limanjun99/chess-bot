#include "bot_state.h"

#include <memory>
#include <optional>

#include "game_state.h"

BotState::BotState(const Config& config, const Lichess& lichess) : config{config}, lichess{lichess} {}

std::optional<std::unique_ptr<BotState>> BotState::handle_challenge(const json& challenge) {
  // Reject all challenges by default.
  if (challenge["status"] != "created") return std::nullopt;
  lichess.handle_challenge(challenge["id"], false);
  return std::nullopt;
}

std::optional<std::unique_ptr<BotState>> BotState::handle_challenge_cancelled(const json& challenge) {
  log_declined_challenge(challenge);
  return std::nullopt;
}

std::optional<std::unique_ptr<BotState>> BotState::handle_challenge_declined(const json& challenge) {
  log_declined_challenge(challenge);
  return std::nullopt;
}

std::optional<std::unique_ptr<BotState>> BotState::handle_game_start(const json& game) {
  return std::make_unique<GameState>(config, lichess, game["gameId"]);
}

std::optional<std::unique_ptr<BotState>> BotState::handle_game_finish(const json& game) {
  log_game_finish(game);
  return std::nullopt;
}

std::optional<std::unique_ptr<BotState>> BotState::handle_null_event() { return std::nullopt; }

// Check if a challenge is issued to us, and its conditions are acceptable.
bool BotState::is_acceptable_challenge(const json& challenge) {
  // Check that challenge is issued to us.
  if (challenge["destUser"].is_null()) return false;
  if (challenge["destUser"]["id"] != config.get_lichess_bot_name()) return false;

  // Check validity of this challenge.
  const bool valid_speed = challenge["speed"] != "classical" && challenge["speed"] != "correspondence";
  const bool valid_variant = challenge["variant"]["key"] == "standard";
  return valid_speed && valid_variant;
}

void BotState::log_declined_challenge(const json& challenge) {
  Logger::get().format_info("Challenge to {} from {} was declined because '{}'.", challenge["destUser"]["id"].dump(),
                            challenge["challenger"]["id"].dump(), challenge["declineReason"].dump());
}

void BotState::log_accepted_challenge(const json& challenge) {
  Logger::get().format_info("Challenge to {} from {} was accepted.", challenge["destUser"]["id"].dump(),
                            challenge["challenger"]["id"].dump());
}

void BotState::log_game_finish(const json& game) { Logger::get().format_info("Game {} ended.", game["id"].dump()); }