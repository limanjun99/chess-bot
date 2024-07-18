#include "uci_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../outputs/id_output.h"
#include "../outputs/uciok_output.h"
#include "command.h"

void UciCommand::execute(EngineCli &engine_cli) const {
  IdOutput engine_info{std::string{engine_cli.get_name()}, std::string{engine_cli.get_author()}};
  engine_cli.write(engine_info);
  engine_cli.write(UciOkOutput{});
}

UciCommand::UciCommand(std::string input_string) : Command{std::move(input_string)} {}

std::string_view UciCommand::get_name() { return "uci"; }