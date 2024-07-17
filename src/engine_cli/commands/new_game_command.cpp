#include "new_game_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../util.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<NewGameCommand>, std::string> NewGameCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<NewGameCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{words.size() == 1 && words[0] == "ucinewgame"};
  if (!valid) {
    return expected::make_unexpected(NewGameCommand::get_usage_info());
  }

  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<NewGameCommand>{new NewGameCommand(std::move(input_string))});
}

std::string_view NewGameCommand::get_usage_info() {
  return "Invalid usage of ucinewgame command. Expected: ucinewgame";
}

void NewGameCommand::execute(EngineCli &engine_cli) const { engine_cli.new_game(); }

NewGameCommand::NewGameCommand(std::string input_string) : Command{std::move(input_string)} {}