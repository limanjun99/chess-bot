
#include "go_command.h"

#include <array>
#include <format>
#include <ranges>
#include <utility>

#include "../engine_cli.h"
#include "../util.h"
#include "chess_engine/uci.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<GoCommand>, std::string> GoCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<GoCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  const bool valid{words.size() >= 1 && words[0] == "go"};
  if (!valid) {
    return expected::make_unexpected(GoCommand::get_usage_info());
  }

  // Using `new` to access private constructor.
  engine::uci::SearchConfig::Builder config_builder{};

  constexpr std::array has_integer_argument{"wtime", "btime", "winc", "binc", "depth", "nodes", "movetime"};
  for (size_t i{1}; i < words.size(); i++) {
    const auto& option{words[i]};

    if (std::ranges::find(has_integer_argument, option) != has_integer_argument.end()) {
      // Parse integer argument.
      if (i + 1 >= words.size()) {
        return expected::make_unexpected(
            std::format("Missing argument for option '{0}' of go command. Expected: {0} <integer>", option));
      }
      ++i;
      const auto& arg_string{words[i]};
      auto argument{command::parsing::parse_integer<int64_t>(arg_string)};
      if (!argument) {
        return expected::make_unexpected(std::format(
            "Invalid argument '{0}' for option '{1}' of go command. Expected: {1} <integer>", arg_string, option));
      }

      if (option == "wtime") config_builder.set_wtime(std::chrono::milliseconds{*argument});
      else if (option == "btime") config_builder.set_btime(std::chrono::milliseconds{*argument});
      else if (option == "winc") config_builder.set_winc(std::chrono::milliseconds{*argument});
      else if (option == "binc") config_builder.set_binc(std::chrono::milliseconds{*argument});
      else if (option == "depth") config_builder.set_depth(*argument);
      else if (option == "nodes") config_builder.set_nodes(*argument);
      else /* if (option == "movetime") */ config_builder.set_movetime(std::chrono::milliseconds{*argument});

    } else if (option == "searchmoves") {
      //! TODO: Implement.
      return expected::make_unexpected(
          std::format("The option '{}' for go command is not currently supported.", option));
    } else if (option == "infinite") {
      config_builder.set_infinite(true);
    } else if (option == "ponder" || option == "movestogo" || option == "mate") {
      return expected::make_unexpected(
          std::format("The option '{}' for go command is not currently supported.", option));
    } else {
      return expected::make_unexpected(std::format("The option '{}' for go command is not recognized.", option));
    }
  }

  return expected::make_expected(
      std::unique_ptr<GoCommand>{new GoCommand(std::move(input_string), config_builder.build())});
}

std::string_view GoCommand::get_usage_info() {
  return "Invalid usage of go command. Refer to the UCI specification for list of options. Expected: go [options]";
}

void GoCommand::execute(EngineCli& engine_cli) const { engine_cli.go(config); }

GoCommand::GoCommand(std::string input_string, engine::uci::SearchConfig config)
    : Command{std::move(input_string)}, config{std::move(config)} {}