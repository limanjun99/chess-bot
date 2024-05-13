#pragma once

#include "bot_state.h"

// Bot is idling, it should periodically challenge other bots, and accept all incoming challenges.
class IdleState : public BotState {
public:
  IdleState(const Config& config, const Lichess& lichess);

  virtual std::optional<std::unique_ptr<BotState>> handle_challenge(const json& challenge) override;

  virtual std::optional<std::unique_ptr<BotState>> handle_null_event() override;
};