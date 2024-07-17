#include "ready_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../outputs/ready_output.h"
#include "../util.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<ReadyCommand>, std::string> ReadyCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<ReadyCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{words.size() == 1 && words[0] == "isready"};
  if (!valid) {
    return expected::make_unexpected(ReadyCommand::get_usage_info());
  }

  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<ReadyCommand>{new ReadyCommand(std::move(input_string))});
}

std::string_view ReadyCommand::get_usage_info() { return "Invalid usage of isready command. Expected: isready"; }

void ReadyCommand::execute(EngineCli &engine_cli) const {
  engine_cli.wait();
  engine_cli.write(ReadyOutput{});
}

ReadyCommand::ReadyCommand(std::string input_string) : Command{std::move(input_string)} {}