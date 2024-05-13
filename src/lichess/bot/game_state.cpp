
#include "game_state.h"

#include "../game_handler.h"
#include "idle_state.h"

// Bot is in a game, it should focus solely on the game.
GameState::GameState(const Config& config, const Lichess& lichess, std::string game_id)
    : BotState{config, lichess}, game_id{std::move(game_id)}, game_thread{[this] { this->spawn_game_handler(); }} {}

std::optional<std::unique_ptr<BotState>> GameState::handle_game_start([[maybe_unused]] const json& game) {
  return std::nullopt;
};

std::optional<std::unique_ptr<BotState>> GameState::handle_game_finish([[maybe_unused]] const json& game) {
  return std::make_unique<IdleState>(config, lichess);
}

GameState::~GameState() { game_thread.join(); }

void GameState::spawn_game_handler() {
  GameHandler game_handler{config, lichess, game_id};
  game_handler.listen();
}