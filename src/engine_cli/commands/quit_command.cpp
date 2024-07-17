#include "quit_command.h"

#include <utility>

#include "../engine_cli.h"
#include "../outputs/id_output.h"
#include "../outputs/uciok_output.h"
#include "../util.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<QuitCommand>, std::string> QuitCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<QuitCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{words.size() == 1 && words[0] == "quit"};
  if (!valid) {
    return expected::make_unexpected(QuitCommand::get_usage_info());
  }

  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<QuitCommand>{new QuitCommand(std::move(input_string))});
}

std::string_view QuitCommand::get_usage_info() { return "Invalid usage of quit command. Expected: quit"; }

void QuitCommand::execute(EngineCli &engine_cli) const { engine_cli.quit(); }

QuitCommand::QuitCommand(std::string input_string) : Command{std::move(input_string)} {}