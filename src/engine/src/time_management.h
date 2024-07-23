#pragma once

#include <chrono>
#include <cstdint>

#include "chess/color.h"
#include "uci.h"

class TimeManagement {
public:
  explicit TimeManagement(const engine::uci::SearchConfig& config, chess::Color player_color);

  // Returns true if cutoff time has been reached.
  // This function is non-const as it might need to update the `timeout_danger` level.
  // If the current time is already known, pass it in to avoid another call to get it.
  [[nodiscard]] bool has_timed_out(
      std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now());

  // How often `has_timed_out` should be called, every time this number of nodes is visited.
  // The returned value is guaranteed to be a power of two.
  // This number will decrease as we get closer to the cutoff time.
  [[nodiscard]] int64_t check_interval() const;

  // Returns the amount of time that has passed since `start_time`.
  // If the current time is already known, pass it in to avoid another call to get it.
  [[nodiscard]] std::chrono::milliseconds time_spent(
      std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now()) const;

  // This should be called at the end of every iteration of iterative deepening.
  // Returns false if there likely won't be sufficient time for the next iteration, else returns true.
  // If the current time is already known, pass it in to avoid another call to get it.
  [[nodiscard]] bool can_continue_iteration(
      std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now());

private:
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point cutoff_time;
  std::chrono::steady_clock::time_point previous_iteration_endpoint;
  enum class TimeoutDanger { Low, Normal, High } timeout_danger;
};