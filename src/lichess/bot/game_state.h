#pragma once

#include <thread>

#include "bot_state.h"

// Bot is in a game, it should focus solely on the game.
class GameState : public BotState {
public:
  GameState(const Config& config, const Lichess& lichess, std::string game_id);

  virtual std::optional<std::unique_ptr<BotState>> handle_game_start(const json& game) override;

  virtual std::optional<std::unique_ptr<BotState>> handle_game_finish(const json& game) override;

  virtual ~GameState();

private:
  std::string game_id;
  std::thread game_thread;

  void spawn_game_handler();
};