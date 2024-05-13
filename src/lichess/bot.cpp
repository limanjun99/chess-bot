#include "bot.h"

#include <array>
#include <random>
#include <string>

#include "bot/bot_state.h"
#include "bot/idle_state.h"
#include "game_handler.h"
#include "logger.h"

Bot::Bot(const Config& config, const Lichess& lichess)
    : config{config}, lichess{lichess}, state{std::make_unique<IdleState>(config, lichess)} {}

void Bot::listen() { lichess.handle_incoming_events(*this); }

void Bot::change_state(std::unique_ptr<BotState> new_state) { state = std::move(new_state); }

void Bot::change_state(std::optional<std::unique_ptr<BotState>> new_state) {
  if (new_state.has_value()) change_state(std::move(*new_state));
}

void Bot::handle_challenge(const json& challenge) { change_state(state->handle_challenge(challenge)); }

void Bot::handle_challenge_cancelled(const json& challenge) {
  change_state(state->handle_challenge_cancelled(challenge));
}

void Bot::handle_challenge_declined(const json& challenge) {
  change_state(state->handle_challenge_declined(challenge));
}

void Bot::handle_game_start(const json& game) { change_state(state->handle_game_start(game)); }

void Bot::handle_game_finish(const json& game) { change_state(state->handle_game_finish(game)); }

void Bot::handle_null_event() { change_state(state->handle_null_event()); }