#include "time_management.h"

#include <chrono>
#include <optional>
#include <utility>

#include "chess/color.h"
#include "uci.h"

std::optional<std::chrono::milliseconds> time_management::decide(const engine::uci::SearchConfig& config,
                                                                 chess::Color player_color) {
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