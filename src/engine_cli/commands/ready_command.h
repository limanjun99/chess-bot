#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "command.h"
#include "parameterless_command.h"

class ReadyCommand : public Command, public command::detail::ParameterlessCommand<ReadyCommand> {
public:
  virtual void execute(EngineCli& engine_cli) const override;

private:
  friend class command::detail::ParameterlessCommand<ReadyCommand>;

  explicit ReadyCommand(std::string input_string);

  [[nodiscard]] static std::string_view get_name();
};