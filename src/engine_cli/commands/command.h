#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class EngineCli;

class Command {
public:
  explicit Command(std::string input_string);

  Command(const Command&) = delete;
  Command(Command&&) = delete;
  Command& operator=(const Command&) = delete;
  Command& operator=(Command&&) = delete;
  virtual ~Command() = default;

  virtual void execute(EngineCli& engine_cli) const = 0;

  const std::string& get_input_string() const;

private:
  std::string input_string;
};

namespace command {

std::expected<std::unique_ptr<Command>, std::string> from_string(std::string command_string);

}
