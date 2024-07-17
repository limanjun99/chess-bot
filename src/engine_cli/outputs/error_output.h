#pragma once

#include <string>

#include "output.h"

class ErrorOutput : public Output {
public:
  explicit ErrorOutput(std::string error_message);

  [[nodiscard]] virtual std::string to_string() const override;

private:
  std::string error_message;
};