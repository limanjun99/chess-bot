#include "stop_command.h"

#include <utility>

#include "../engine_cli.h"
#include "command.h"

void StopCommand::execute(EngineCli &engine_cli) const { engine_cli.stop(); }

StopCommand::StopCommand(std::string input_string) : Command{std::move(input_string)} {}

std::string_view StopCommand::get_name() { return "stop"; }