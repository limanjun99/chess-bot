#pragma once

#include <string>

#include "output.h"

class UciOkOutput : public Output {
public:
  explicit UciOkOutput() = default;

  [[nodiscard]] virtual std::string to_string() const override;
};