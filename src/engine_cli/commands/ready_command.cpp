#include "ready_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../outputs/ready_output.h"
#include "command.h"

void ReadyCommand::execute(EngineCli &engine_cli) const {
  engine_cli.wait();
  engine_cli.write(ReadyOutput{});
}

ReadyCommand::ReadyCommand(std::string input_string) : Command{std::move(input_string)} {}

std::string_view ReadyCommand::get_name() { return "isready"; }