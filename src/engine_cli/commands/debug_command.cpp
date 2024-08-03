#include "debug_command.h"

#include <utility>

#include "../engine_cli.h"
#include "command.h"
#include "parsing.h"
#include "util/expected.h"

std::expected<std::unique_ptr<DebugCommand>, std::string> DebugCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<DebugCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{[&words]() {
    if (words.size() != 2) return false;
    if (words[0] != "debug") return false;
    if (words[1] != "on" && words[1] != "off") return false;
    return true;
  }()};
  if (!valid) {
    return expected::make_unexpected(DebugCommand::get_usage_info());
  }

  const bool debug_mode{words[1] == "on"};
  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<DebugCommand>{new DebugCommand(std::move(input_string), debug_mode)});
}

std::string_view DebugCommand::get_usage_info() { return "Invalid usage of debug command. Expected: debug [on | off]"; }

void DebugCommand::execute(EngineCli& engine_cli) const { engine_cli.set_debug(debug_mode); }

bool DebugCommand::get_debug_mode() const { return debug_mode; }

DebugCommand::DebugCommand(std::string input_string, bool debug_mode)
    : Command{std::move(input_string)}, debug_mode{debug_mode} {}