#include "position_command.h"

#include <iterator>
#include <ranges>
#include <utility>

#include "../engine_cli.h"
#include "../util.h"
#include "chess/uci.h"
#include "command.h"
#include "parsing.h"

std::expected<std::unique_ptr<PositionCommand>, std::string> PositionCommand::from_string(std::string input_string) {
  using expected = util::expected<std::unique_ptr<PositionCommand>, std::string>;

  const auto words{command::parsing::split_string(input_string)};

  if (words.size() < 2 || words[0] != "position" || (words[1] != "fen" && words[1] != "startpos")) {
    return expected::make_unexpected(PositionCommand::get_usage_info());
  }

  //! TODO (low priority): Add input validation for the fen and moves.
  // Is low priority for now as this cli will only be used by correctly implemented uci programs.

  // Setup initial position from startpos / given fen.
  chess::Board position{[&input_string, &words]() {
    if (words[1] == "startpos") return chess::Board::initial();

    const size_t fen_start{input_string.find("fen") + 3};
    size_t fen_end{input_string.find_last_of("moves")};
    if (fen_end == std::string::npos) fen_end = input_string.size();

    return chess::Board::from_fen(std::string_view{input_string}.substr(fen_start, fen_end - fen_start));
  }()};

  // Apply moves to the given position.
  const auto is_not_move_delimiter{[](const auto& word) { return word != "moves"; }};
  for (const auto& uci_move_string : words | std::views::drop_while(is_not_move_delimiter) | std::views::drop(1)) {
    chess::Move move{chess::uci::move(uci_move_string, position)};
    position = position.apply_move(move);
  }

  // Using `new` to access private constructor.
  return expected::make_expected(
      std::unique_ptr<PositionCommand>{new PositionCommand(std::move(input_string), std::move(position))});
}

std::string_view PositionCommand::get_usage_info() {
  return "Invalid usage of position command. "
         "Expected: position [fen <fenstring> | startpos ]  moves <move1> .... <movei>";
}

void PositionCommand::execute(EngineCli& engine_cli) const { engine_cli.set_position(position); }

const chess::Board& PositionCommand::get_position() const { return position; }

PositionCommand::PositionCommand(std::string input_string, chess::Board position)
    : Command{std::move(input_string)}, position{std::move(position)} {}