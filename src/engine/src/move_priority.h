#pragma once

#include <cstdint>

#include "chess/color.h"
#include "chess/move.h"
#include "heuristics.h"

// Higher priority moves should be searched first.
class MovePriority {
public:
  explicit constexpr MovePriority(int32_t priority);

  constexpr auto operator<=>(const MovePriority& other) const;

  // Returns the priority level of a move.
  static MovePriority evaluate(const chess::Move& move, int32_t depth_left, const chess::Move& hash_move,
                               chess::Color player_color, Heuristics& heuristics);

  // Returns the priority level of a quiescence move.
  static MovePriority evaluate_quiescence(const chess::Move& move);

private:
  int32_t priority;
};

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

constexpr MovePriority::MovePriority(int32_t priority) : priority{priority} {}

constexpr auto MovePriority::operator<=>(const MovePriority& other) const { return priority <=> other.priority; }