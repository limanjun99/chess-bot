#include "command.h"

#include <cctype>

#include "../util.h"
#include "debug_command.h"
#include "go_command.h"
#include "new_game_command.h"
#include "parsing.h"
#include "position_command.h"
#include "quit_command.h"
#include "ready_command.h"
#include "stop_command.h"
#include "uci_command.h"

Command::Command(std::string input_string) : input_string{std::move(input_string)} {}

const std::string& Command::get_input_string() const { return input_string; }

std::expected<std::unique_ptr<Command>, std::string> command::from_string(std::string command_string) {
  using expected = util::expected<std::unique_ptr<Command>, std::string>;

  const auto first_word{command::parsing::first_word(command_string)};

  if (first_word == "uci") {
    return UciCommand::from_string(std::move(command_string));
  } else if (first_word == "debug") {
    return DebugCommand::from_string(std::move(command_string));
  } else if (first_word == "ucinewgame") {
    return NewGameCommand::from_string(std::move(command_string));
  } else if (first_word == "position") {
    return PositionCommand::from_string(std::move(command_string));
  } else if (first_word == "go") {
    return GoCommand::from_string(std::move(command_string));
  } else if (first_word == "stop") {
    return StopCommand::from_string(std::move(command_string));
  } else if (first_word == "isready") {
    return ReadyCommand::from_string(std::move(command_string));
  } else if (first_word == "quit") {
    return QuitCommand::from_string(std::move(command_string));
  }

  return expected::make_unexpected(std::format("Unrecognized command '{}'", first_word));
}