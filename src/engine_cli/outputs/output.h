#pragma once

#include <ostream>
#include <string>

class EngineCli;

class Output {
public:
  Output(const Output&) = delete;
  Output(Output&&) = delete;
  Output& operator=(const Output&) = delete;
  Output& operator=(Output&&) = delete;
  virtual ~Output() = default;

  // Converts the output to a string.
  [[nodiscard]] virtual std::string to_string() const = 0;

protected:
  Output() = default;
};

std::ostream& operator<<(std::ostream& os, const Output& output);