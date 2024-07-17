#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "chess_engine/uci.h"
#include "command.h"

class GoCommand : public Command {
public:
  // Constructs a GoCommand from an input string, or returns an error string if the input is invalid.
  [[nodiscard]] static std::expected<std::unique_ptr<GoCommand>, std::string> from_string(std::string input_string);

  [[nodiscard]] static std::string_view get_usage_info();

  virtual void execute(EngineCli& engine_cli) const override;

private:
  engine::uci::SearchConfig config;

  explicit GoCommand(std::string input_string, engine::uci::SearchConfig config);
};