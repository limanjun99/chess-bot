#pragma once

#include <cstdint>

#include "chess/color.h"
#include "chess/move.h"
#include "heuristics.h"

// Higher priority moves should be searched first.
class MovePriority {
public:
  constexpr auto operator<=>(const MovePriority& other) const = default;

  // Returns the priority level of a move.
  [[nodiscard]] static MovePriority evaluate(const chess::Move& move, int32_t depth_left, const chess::Move& hash_move,
                                             chess::Color player_color, Heuristics& heuristics);

  // Returns the priority level of a quiescence move.
  [[nodiscard]] static MovePriority evaluate_quiescence(const chess::Move& move);

private:
  explicit constexpr MovePriority(int32_t priority);

  int32_t priority;
};

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

constexpr MovePriority::MovePriority(int32_t priority) : priority{priority} {}