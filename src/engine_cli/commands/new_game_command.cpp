#include "new_game_command.h"

#include <utility>

#include "../engine_cli.h"
#include "command.h"

void NewGameCommand::execute(EngineCli &engine_cli) const { engine_cli.new_game(); }

NewGameCommand::NewGameCommand(std::string input_string) : Command{std::move(input_string)} {}

std::string_view NewGameCommand::get_name() { return "ucinewgame"; }