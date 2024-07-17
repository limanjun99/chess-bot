#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include "chess/move.h"

namespace engine::uci {

class SearchConfig {
public:
  //! TODO: What is the `ponder` option?
  //! TODO: `movestogo` is ignored for now. Check again if it is something that should be added.
  //! TODO: `mate` is ignored for now.

  std::vector<chess::Move> search_moves;
  std::optional<std::chrono::milliseconds> wtime;
  std::optional<std::chrono::milliseconds> winc;
  std::optional<std::chrono::milliseconds> btime;
  std::optional<std::chrono::milliseconds> binc;
  std::optional<int32_t> depth;
  std::optional<int64_t> nodes;
  std::optional<std::chrono::milliseconds> movetime;
  bool infinite;

  class Builder;

private:
  explicit SearchConfig();
};

class SearchConfig::Builder {
public:
  explicit Builder();

  void add_search_moves(chess::Move move);
  void set_wtime(std::chrono::milliseconds time);
  void set_winc(std::chrono::milliseconds increment);
  void set_btime(std::chrono::milliseconds time);
  void set_binc(std::chrono::milliseconds increment);
  void set_depth(int32_t depth);
  void set_nodes(int64_t nodes);
  void set_movetime(std::chrono::milliseconds time);
  void set_infinite(bool infinite);

  // Returns the built configuration object.
  [[nodiscard]] SearchConfig build();

private:
  SearchConfig config;
};

}  // namespace engine::uci