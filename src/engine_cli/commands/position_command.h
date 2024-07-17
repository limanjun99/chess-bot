#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "chess/board.h"
#include "command.h"

class PositionCommand : public Command {
public:
  // Constructs a PositionCommand from an input string, or returns an error string if the input is invalid.
  [[nodiscard]] static std::expected<std::unique_ptr<PositionCommand>, std::string> from_string(
      std::string input_string);

  [[nodiscard]] static std::string_view get_usage_info();

  virtual void execute(EngineCli& engine_cli) const override;

  [[nodiscard]] const chess::Board& get_position() const;

private:
  chess::Board position;

  explicit PositionCommand(std::string input_string, chess::Board position);
};