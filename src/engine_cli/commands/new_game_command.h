#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "command.h"
#include "parameterless_command.h"

class NewGameCommand : public Command, public command::detail::ParameterlessCommand<NewGameCommand> {
public:
  virtual void execute(EngineCli& engine_cli) const override;

private:
  friend class command::detail::ParameterlessCommand<NewGameCommand>;

  explicit NewGameCommand(std::string input_string);

  [[nodiscard]] static std::string_view get_name();
};