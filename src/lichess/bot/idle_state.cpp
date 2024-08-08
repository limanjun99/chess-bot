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
  return config.should_issue_challenges() &&
         std::chrono::steady_clock::now() - idle_since > config.get_challenge_interval();
}

std::optional<std::unique_ptr<BotState>> IdleState::issue_challenge() {
  idle_since = std::chrono::steady_clock::now();

  // List of initial time and increment (in seconds) to choose from.
  //! TODO: Add no-increment clocks once we handle network latency.
  const std::array<std::tuple<int, int, std::string>, 3> clocks = {{
      {120, 1, "bullet"},  // 2+1 Bullet
      {180, 2, "blitz"},   // 3+2 Blitz
      {300, 3, "blitz"},   // 5+3 Blitz
  }};
  std::mt19937 rng{static_cast<std::mt19937::result_type>(std::time(NULL))};
  const int clock_choice = std::uniform_int_distribution<>{0, static_cast<int>(clocks.size()) - 1}(rng);
  const auto [clock_time, clock_increment, time_control] = clocks[clock_choice];

  auto my_profile = lichess.get_my_profile();
  const int my_rating = my_profile["perfs"][time_control]["rating"];
  const int rating_range = 300;  // Only play against bots with a rating within +- 300 of myself.

  // Pick a random bot from the list of online bots.
  std::vector<json> online_bots = lichess.get_online_bots(300);
  std::vector<std::string> valid_bots;
  for (const json& bot : online_bots) {
    if (bot["username"] == config.get_lichess_bot_name()) continue;
    if (!bot["perfs"].contains(time_control)) continue;
    const int bot_rating = bot["perfs"][time_control]["rating"];
    if (bot_rating > my_rating + rating_range || bot_rating < my_rating - rating_range) continue;
    valid_bots.push_back(bot["username"]);
  }

  if (valid_bots.empty()) {
    Logger::get().warn("No valid online bots found to challenge.");
    return std::nullopt;
  }
  const size_t username_index = (std::uniform_int_distribution<size_t>{1, valid_bots.size()}(rng)) - 1;
  const std::string& username = valid_bots[username_index];

  // Issue a challenge to the bot.
  Logger::get().format_info("Issuing challenge to {}.", username);
  if (auto challenge_id = lichess.issue_challenge(username, true, clock_time, clock_increment)) {
    return std::make_unique<ChallengeIssuedState>(config, lichess);
  } else {
    Logger::get().format_warn("Challenge to {} failed.", username);
    return std::nullopt;
  }
}