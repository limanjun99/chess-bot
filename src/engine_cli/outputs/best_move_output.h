#pragma once

#include <string>

#include "chess/move.h"
#include "output.h"

class BestMoveOutput : public Output {
public:
  explicit BestMoveOutput(chess::Move move);

  [[nodiscard]] virtual std::string to_string() const override;

private:
  chess::Move move;
};