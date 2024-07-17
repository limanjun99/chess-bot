#include "stop_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../util.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<StopCommand>, std::string> StopCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<StopCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{words.size() == 1 && words[0] == "stop"};
  if (!valid) {
    return expected::make_unexpected(StopCommand::get_usage_info());
  }

  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<StopCommand>{new StopCommand(std::move(input_string))});
}

std::string_view StopCommand::get_usage_info() { return "Invalid usage of stop command. Expected: stop"; }

void StopCommand::execute(EngineCli &engine_cli) const { engine_cli.stop(); }

StopCommand::StopCommand(std::string input_string) : Command{std::move(input_string)} {}