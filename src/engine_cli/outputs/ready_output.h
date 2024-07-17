#pragma once

#include <string>

#include "output.h"

class ReadyOutput : public Output {
public:
  explicit ReadyOutput() = default;

  [[nodiscard]] virtual std::string to_string() const override;
};