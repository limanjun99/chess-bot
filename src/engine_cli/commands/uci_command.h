#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "command.h"
#include "parameterless_command.h"

class UciCommand : public Command, public command::detail::ParameterlessCommand<UciCommand> {
public:
  virtual void execute(EngineCli& engine_cli) const override;

private:
  friend class command::detail::ParameterlessCommand<UciCommand>;

  explicit UciCommand(std::string input_string);

  [[nodiscard]] static std::string_view get_name();
};