#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include "chess/move.h"

namespace engine::uci {

struct SearchConfig {
  //! TODO: What is the `ponder` option?
  //! TODO: `movestogo` is ignored for now. Check again if it is something that should be added.
  //! TODO: `mate` is ignored for now.

  explicit SearchConfig();

  // A wrapper for `SearchConfig{}.set_depth(depth)`.
  static SearchConfig from_depth(int32_t depth);
  // A wrapper for `SearchConfig{}.set_movetime(time)`.
  static SearchConfig from_movetime(std::chrono::milliseconds time);

  std::vector<chess::Move> search_moves;
  std::optional<std::chrono::milliseconds> wtime;
  std::optional<std::chrono::milliseconds> winc;
  std::optional<std::chrono::milliseconds> btime;
  std::optional<std::chrono::milliseconds> binc;
  std::optional<int32_t> depth;
  std::optional<int64_t> nodes;
  std::optional<std::chrono::milliseconds> movetime;
  bool infinite;

  SearchConfig& add_search_move(chess::Move move);
  SearchConfig& set_wtime(std::chrono::milliseconds time);
  SearchConfig& set_winc(std::chrono::milliseconds increment);
  SearchConfig& set_btime(std::chrono::milliseconds time);
  SearchConfig& set_binc(std::chrono::milliseconds increment);
  SearchConfig& set_depth(int32_t depth);
  SearchConfig& set_nodes(int64_t nodes);
  SearchConfig& set_movetime(std::chrono::milliseconds time);
  SearchConfig& set_infinite(bool infinite);
};

}  // namespace engine::uci