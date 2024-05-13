#include "idle_state.h"

#include <random>
#include <string>
#include <vector>

#include "challenge_accepted_state.h"
#include "challenge_issued_state.h"

IdleState::IdleState(const Config& config, const Lichess& lichess)
    : BotState{config, lichess}, idle_since{std::chrono::steady_clock::now()} {}

std::optional<std::unique_ptr<BotState>> IdleState::handle_challenge(const json& challenge) {
  if (challenge["status"] != "created") return std::nullopt;

  if (!is_acceptable_challenge(challenge)) {
    // Decline challenge.
    lichess.handle_challenge(challenge["id"], false);
    log_declined_challenge(challenge);
    return std::nullopt;
  }

  // Accept challenge.
  bool accepted = lichess.handle_challenge(challenge["id"], true);
  if (!accepted) return std::nullopt;

  log_accepted_challenge(challenge);
  return std::make_unique<ChallengeAcceptedState>(config, lichess);
}

std::optional<std::unique_ptr<BotState>> IdleState::handle_null_event() {  // todo
  if (is_ready_to_challenge()) {
    return issue_challenge();
  }
  return std::nullopt;
}

bool IdleState::is_ready_to_challenge() const {
  return config.should_issue_challenges() && std::chrono::steady_clock::now() - idle_since > std::chrono::minutes{5};
}

std::optional<std::unique_ptr<BotState>> IdleState::issue_challenge() {
  idle_since = std::chrono::steady_clock::now();

  // List of initial time and increment (in seconds) to choose from.
  //! TODO: Add no-increment clocks once we handle network latency.
  constexpr std::array<std::pair<int, int>, 3> clocks = {{
      {120, 1},  // 2+1 Bullet
                 // {180, 2},  // 3+2 Blitz
                 // {300, 3},  // 5+3 Blitz
  }};
  std::mt19937 rng{static_cast<std::mt19937::result_type>(std::time(NULL))};
  const int clock_choice = std::uniform_int_distribution<>{0, clocks.size() - 1}(rng);
  const int clock_time = clocks[clock_choice].first;
  const int clock_increment = clocks[clock_choice].second;

  // Pick a random bot from the list of online bots.
  std::vector<std::string> online_bots = lichess.get_online_bots(150);
  if (online_bots.empty()) {
    Logger::warn() << "No online bots found to challenge.\n";
    Logger::flush();
    return std::nullopt;
  }
  const size_t username_index = (std::uniform_int_distribution<size_t>{1, online_bots.size()}(rng)) - 1;
  const std::string& username = online_bots[username_index];

  // Issue a challenge to the bot.
  Logger::info() << "Issuing challenge to " << username << "\n";
  Logger::flush();
  if (auto challenge_id = lichess.issue_challenge(username, true, clock_time, clock_increment)) {
    return std::make_unique<ChallengeIssuedState>(config, lichess);
  } else {
    Logger::warn() << "Challenge to " << username << " failed\n";
    Logger::flush();
    return std::nullopt;
  }
}