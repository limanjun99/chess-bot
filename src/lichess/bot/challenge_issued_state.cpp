#include "challenge_issued_state.h"

#include "idle_state.h"

ChallengeIssuedState::ChallengeIssuedState(const Config& config, const Lichess& lichess)
    : BotState{config, lichess}, issued_since{std::chrono::steady_clock::now()} {}

// Bot has issued a challenge and is waiting for its result.
std::optional<std::unique_ptr<BotState>> ChallengeIssuedState::handle_challenge_cancelled(const json& challenge) {
  log_declined_challenge(challenge);
  return std::make_unique<IdleState>(config, lichess);
}

std::optional<std::unique_ptr<BotState>> ChallengeIssuedState::handle_challenge_declined(const json& challenge) {
  log_declined_challenge(challenge);
  return std::make_unique<IdleState>(config, lichess);
}

std::optional<std::unique_ptr<BotState>> ChallengeIssuedState::handle_null_event() {
  // Challenges expire after 20s.
  if (std::chrono::steady_clock::now() - issued_since > std::chrono::seconds{20}) {
    Logger::get().info("Challenge timed out, back to idling");
    return std::make_unique<IdleState>(config, lichess);
  }
  return std::nullopt;
}
