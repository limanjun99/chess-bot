#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "command.h"
#include "parameterless_command.h"

class StopCommand : public Command, public command::detail::ParameterlessCommand<StopCommand> {
public:
  virtual void execute(EngineCli& engine_cli) const override;

private:
  friend class command::detail::ParameterlessCommand<StopCommand>;

  explicit StopCommand(std::string input_string);

  [[nodiscard]] static std::string_view get_name();
};