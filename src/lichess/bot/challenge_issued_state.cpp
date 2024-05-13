#include "challenge_issued_state.h"

#include "game_state.h"
#include "idle_state.h"

ChallengeIssuedState::ChallengeIssuedState(const Config& config, const Lichess& lichess) : BotState{config, lichess} {}

// Bot has issued a challenge and is waiting for its result.
std::optional<std::unique_ptr<BotState>> ChallengeIssuedState::handle_challenge_cancelled(const json& challenge) {
  log_declined_challenge(challenge);
  return std::make_unique<IdleState>(config, lichess);
}

std::optional<std::unique_ptr<BotState>> ChallengeIssuedState::handle_challenge_declined(const json& challenge) {
  log_declined_challenge(challenge);
  return std::make_unique<IdleState>(config, lichess);
}