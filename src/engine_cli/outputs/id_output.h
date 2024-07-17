#pragma once

#include <string>

#include "output.h"

class IdOutput : public Output {
public:
  explicit IdOutput(std::string name, std::string author);

  [[nodiscard]] virtual std::string to_string() const override;

private:
  std::string name;
  std::string author;
};