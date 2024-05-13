#pragma once

#include "bot_state.h"

// Bot has issued a challenge and is waiting for its result.
class ChallengeIssuedState : public BotState {
public:
  ChallengeIssuedState(const Config& config, const Lichess& lichess);

  virtual std::optional<std::unique_ptr<BotState>> handle_challenge_cancelled(const json& challenge) override;

  virtual std::optional<std::unique_ptr<BotState>> handle_challenge_declined(const json& challenge) override;
};