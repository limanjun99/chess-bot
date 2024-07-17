#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "command.h"

class NewGameCommand : public Command {
public:
  // Constructs a NewGameCommand from an input string, or returns an error string if the input is invalid.
  [[nodiscard]] static std::expected<std::unique_ptr<NewGameCommand>, std::string> from_string(
      std::string input_string);

  [[nodiscard]] static std::string_view get_usage_info();

  virtual void execute(EngineCli& engine_cli) const override;

private:
  explicit NewGameCommand(std::string input_string);
};