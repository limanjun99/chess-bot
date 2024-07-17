#include "uci_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../outputs/id_output.h"
#include "../outputs/uciok_output.h"
#include "../util.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<UciCommand>, std::string> UciCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<UciCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{words.size() == 1 && words[0] == "uci"};
  if (!valid) {
    return expected::make_unexpected(UciCommand::get_usage_info());
  }

  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<UciCommand>{new UciCommand(std::move(input_string))});
}

std::string_view UciCommand::get_usage_info() { return "Invalid usage of uci command. Expected: uci"; }

void UciCommand::execute(EngineCli &engine_cli) const {
  IdOutput engine_info{std::string{engine_cli.get_name()}, std::string{engine_cli.get_author()}};
  engine_cli.write(engine_info);
  engine_cli.write(UciOkOutput{});
}

UciCommand::UciCommand(std::string input_string) : Command{std::move(input_string)} {}