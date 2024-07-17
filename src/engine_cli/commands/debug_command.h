#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "command.h"

class DebugCommand : public Command {
public:
  // Constructs a DebugCommand from an input string, or returns an error string if the input is invalid.
  [[nodiscard]] static std::expected<std::unique_ptr<DebugCommand>, std::string> from_string(std::string input_string);

  [[nodiscard]] static std::string_view get_usage_info();

  virtual void execute(EngineCli& engine_cli) const override;

  [[nodiscard]] bool get_debug_mode() const;

private:
  bool debug_mode;

  explicit DebugCommand(std::string input_string, bool debug_mode);
};