#include "bot.h"

#include <array>
#include <random>
#include <string>

#include "game_handler.h"
#include "logger.h"

Bot::Bot(const Config& config, const Lichess& lichess)
    : config{config},
      lichess{lichess},
      state_from{std::chrono::steady_clock::now()},
      state{State::Idle},
      gen{static_cast<unsigned int>(time(NULL))},
      online_bots{},
      online_bots_from{std::chrono::steady_clock::now() - std::chrono::years{1}} {}

void Bot::listen() { lichess.handle_incoming_events(*this); }

void Bot::change_state(State new_state) {
  state_from = std::chrono::steady_clock::now();
  state = new_state;
}

void Bot::handle_challenge(const json& challenge) {
  // Ignore all events that are not other players challenging us.
  if (challenge["status"] != "created" || challenge["challenger"]["id"] == config.get_lichess_bot_name()) return;

  // Check validity of this challenge.
  const bool valid_speed = challenge["speed"] != "classical" && challenge["speed"] != "correspondence";
  const bool valid_variant = challenge["variant"]["key"] == "standard";
  const bool valid_state = state == State::Idle;

  if (valid_speed && valid_variant && valid_state) {
    // Accept challenge.
    bool accepted = lichess.handle_challenge(challenge["id"], true);
    if (accepted) change_state(State::InGame);
    Logger::info() << "Accepted challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
    Logger::flush();
  } else {
    // Decline challenge.
    lichess.handle_challenge(challenge["id"], false);
    Logger::info() << "Declined challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
    Logger::flush();
  }
}

//! TODO: What is the difference between a cancelled challenge and a declined challenge?
//! For now both their handler functions are identical.

void Bot::handle_challenge_cancelled(const json& challenge) {
  Logger::info() << "Challenge to " << challenge["destUser"]["name"] << " was declined because '"
                 << challenge["declineReason"] << "'\n";
  Logger::flush();
  if (state == State::IssueChallenge) change_state(State::Idle);
}

void Bot::handle_challenge_declined(const json& challenge) {
  Logger::info() << "Challenge to " << challenge["destUser"]["name"] << " was declined because '"
                 << challenge["declineReason"] << "'\n";
  Logger::flush();
  if (state == State::IssueChallenge) change_state(State::Idle);
}

void Bot::handle_game_start(const json& game) {
  change_state(State::InGame);
  GameHandler game_handler{config, lichess, game["gameId"]};
  game_handler.listen();
  change_state(State::Idle);
}

void Bot::handle_game_finish(const json& game) {
  //! TODO: It looks like game end is already handled by the GameHandler.
  //! Find out if we actually need to do anything here.
}

void Bot::handle_null_event() {
  //! TODO: Confirm that Lichess API does not update us when a challenge expires (after 20s).
  //! TODO: Figure out why challenges are not actually expiring (bots can accept them long after 20s,
  //! resulting in abortions as we have entered another game).
  if (state == State::IssueChallenge && std::chrono::steady_clock::now() - state_from > std::chrono::seconds{20}) {
    // Challenge has timed out, try to cancel the challenge and switch back to idle.
    Logger::info() << "Challenge timed out.\n";
    Logger::flush();
    lichess.cancel_challenge(issued_challenge_id);
    change_state(State::Idle);
  } else if (is_ready_to_refresh_bots()) {
    refresh_online_bots();
  } else if (is_ready_to_challenge()) {
    issue_challenge();
  }
}

bool Bot::is_ready_to_challenge() const {
  // The bot should issue challenges once it has been idling for more than 1 minute.
  if (!config.should_issue_challenges()) return false;
  return state == State::Idle && std::chrono::steady_clock::now() - state_from > std::chrono::minutes{1};
}

bool Bot::is_ready_to_refresh_bots() const {
  // The bot should refresh its list of online bots every 15 minutes.
  if (!config.should_issue_challenges()) return false;
  return state == State::Idle && std::chrono::steady_clock::now() - online_bots_from > std::chrono::minutes{15};
}

void Bot::issue_challenge() {
  // List of initial time and increment (in seconds) to choose from.
  //! TODO: Add no-increment clocks once we handle network latency.
  constexpr std::array<std::pair<int, int>, 3> clocks = {{
      {120, 1},  // 2+1 Bullet
      {180, 2},  // 3+2 Blitz
      {300, 3},  // 5+3 Blitz
  }};
  const int clock_choice = std::uniform_int_distribution<>{0, clocks.size() - 1}(gen);
  const int clock_time = clocks[clock_choice].first;
  const int clock_increment = clocks[clock_choice].second;

  // Pick a random bot from the list of online bots.
  if (online_bots.empty()) {
    Logger::warn() << "No online bots found to challenge.\n";
    Logger::flush();
    // We do a quick switch here to record that we tried to send a challenge,
    // so that we do not immediately try sending challenges again before the waiting period is up.
    change_state(State::IssueChallenge);
    change_state(State::Idle);
    return;
  }
  const size_t username_index = (std::uniform_int_distribution<size_t>{1, online_bots.size()}(gen)) - 1;
  const std::string& username = online_bots[username_index];

  // Issue a challenge to the bot.
  change_state(State::IssueChallenge);
  Logger::info() << "Issuing challenge to " << username << "\n";
  Logger::flush();
  if (auto challenge_id = lichess.issue_challenge(username, true, clock_time, clock_increment)) {
    issued_challenge_id = *challenge_id;
  } else {
    Logger::warn() << "Challenge to " << username << " failed\n";
    Logger::flush();
  }
}

void Bot::refresh_online_bots() {
  Logger::info() << "Getting list of online bots\n";
  Logger::flush();
  online_bots_from = std::chrono::steady_clock::now();
  online_bots = lichess.get_online_bots(150);
  if (online_bots.empty()) {
    Logger::warn() << "Did not manage to find an online bot to challenge.\n";
    Logger::flush();
  }
}