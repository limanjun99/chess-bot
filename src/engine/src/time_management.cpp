#include "time_management.h"

#include <chrono>
#include <optional>
#include <utility>

#include "chess/color.h"
#include "uci.h"

// Decide the maximum amount of time to spend searching on the next move.
// Returns std::nullopt if the search should be indefinite.
std::optional<std::chrono::milliseconds> decide(const engine::uci::SearchConfig& config, chess::Color player_color) {
  // Always defer to the configured movetime if there is one.
  if (config.movetime) return config.movetime.value();

  const auto get_times = [&config](chess::Color color) {
    if (color == chess::Color::White) return std::make_pair(config.wtime, config.winc);
    else return std::make_pair(config.btime, config.binc);
  };
  const auto& [my_time, my_inc] = get_times(player_color);

  // Search indefinitely if no time is provided.
  if (!my_time) return std::nullopt;

  //! TODO: Improve time management.
  return my_time.value() / 200 + my_inc.value_or(std::chrono::milliseconds{0});
}

TimeManagement::TimeManagement(const engine::uci::SearchConfig& config, chess::Color player_color)
    : start_time{std::chrono::steady_clock::now()},
      cutoff_time{std::chrono::steady_clock::time_point::max()},
      previous_iteration_endpoint{start_time},
      timeout_danger{TimeoutDanger::Low} {
  if (const auto move_time{decide(config, player_color)}) {
    // 2ms is a safety buffer to ensure we do not exceed the actual cutoff time.
    cutoff_time = start_time + move_time.value() - std::chrono::milliseconds{2};
  }
  std::ignore = has_timed_out(start_time);  // Update `timeout_danger`.
}

bool TimeManagement::has_timed_out(std::chrono::steady_clock::time_point current_time) {
  if (current_time >= cutoff_time) return true;

  if (timeout_danger == TimeoutDanger::Low) {
    if (current_time + std::chrono::milliseconds{100} >= cutoff_time) timeout_danger = TimeoutDanger::Normal;
  }
  if (timeout_danger == TimeoutDanger::Normal) {
    if (current_time + std::chrono::milliseconds{10} >= cutoff_time) timeout_danger = TimeoutDanger::High;
  }

  return false;
}

int64_t TimeManagement::check_interval() const {
  // There is no particular reason for these values, other than that they seem to work well enough.
  // The only notable feature is that they are powers of two for faster modulo checking.
  // These may need to be adjusted if the engine speed changes.
  switch (timeout_danger) {
    case TimeoutDanger::Low:
      return 1 << 17;
    case TimeoutDanger::Normal:
      return 1 << 14;
    case TimeoutDanger::High:
      return 1 << 9;
    default:
      std::unreachable();
  }
}

std::chrono::milliseconds TimeManagement::time_spent(std::chrono::steady_clock::time_point current_time) const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
}

bool TimeManagement::can_continue_iteration(std::chrono::steady_clock::time_point current_time) {
  const auto iteration_time_spent{current_time - previous_iteration_endpoint};
  previous_iteration_endpoint = current_time;
  // Assume that the next iteration will take at least 1.5x as much time as the previous iteration.
  return current_time + iteration_time_spent * 3 / 2 <= cutoff_time;
}