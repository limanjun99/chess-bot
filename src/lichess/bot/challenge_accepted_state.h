#pragma once

#include "bot_state.h"

// Bot has accepted a challenge, waiting for its game to start.
class ChallengeAcceptedState : public BotState {
public:
  ChallengeAcceptedState(const Config& config, const Lichess& lichess);
};