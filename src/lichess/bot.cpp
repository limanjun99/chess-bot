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
      gen{static_cast<unsigned int>(time(NULL))} {}

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
  }
  if (is_ready_to_challenge()) {
    issue_challenge();
  }
}

bool Bot::is_ready_to_challenge() {
  // The bot should issue challenges once it has been idling for more than 5 seconds.
  return state == State::Idle && std::chrono::steady_clock::now() - state_from > std::chrono::seconds{5};
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
  const std::vector<std::string> usernames = lichess.get_online_bots(150);
  if (usernames.empty()) {
    Logger::warn() << "Did not manage to find an online bot to challenge.\n";
    Logger::flush();
    return;
  }
  const size_t username_index = (std::uniform_int_distribution<size_t>{1, usernames.size()}(gen)) - 1;
  const std::string& username = usernames[username_index];

  // Issue a challenge to the bot.
  change_state(State::IssueChallenge);
  Logger::info() << "Issuing challenge to " << username << "\n";
  Logger::flush();
  if (auto challenge_id = lichess.issue_challenge(username, false, clock_time, clock_increment)) {
    issued_challenge_id = *challenge_id;
  } else {
    Logger::warn() << "Challenge to " << username << " failed\n";
    Logger::flush();
  }
}