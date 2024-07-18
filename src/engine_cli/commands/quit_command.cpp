#include "quit_command.h"

#include <utility>

#include "../engine_cli.h"
#include "command.h"

void QuitCommand::execute(EngineCli &engine_cli) const { engine_cli.quit(); }

QuitCommand::QuitCommand(std::string input_string) : Command{std::move(input_string)} {}

std::string_view QuitCommand::get_name() { return "quit"; }