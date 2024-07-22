#include "uci.h"

namespace engine::uci {

SearchConfig::SearchConfig()
    : search_moves{},
      wtime{std::nullopt},
      winc{std::nullopt},
      btime{std::nullopt},
      binc{std::nullopt},
      depth{std::nullopt},
      nodes{std::nullopt},
      movetime{std::nullopt},
      infinite{false} {}

SearchConfig SearchConfig::from_depth(int32_t new_depth) { return SearchConfig{}.set_depth(new_depth); }

SearchConfig SearchConfig::from_movetime(std::chrono::milliseconds new_time) {
  return SearchConfig{}.set_movetime(new_time);
}

SearchConfig& SearchConfig::add_search_move(chess::Move new_move) {
  search_moves.push_back(new_move);
  return *this;
}

SearchConfig& SearchConfig::set_wtime(std::chrono::milliseconds new_time) {
  wtime = new_time;
  return *this;
}

SearchConfig& SearchConfig::set_winc(std::chrono::milliseconds new_increment) {
  winc = new_increment;
  return *this;
}

SearchConfig& SearchConfig::set_btime(std::chrono::milliseconds new_time) {
  btime = new_time;
  return *this;
}

SearchConfig& SearchConfig::set_binc(std::chrono::milliseconds new_increment) {
  binc = new_increment;
  return *this;
}

SearchConfig& SearchConfig::set_depth(int new_depth) {
  depth = new_depth;
  return *this;
}

SearchConfig& SearchConfig::set_nodes(int64_t new_nodes) {
  nodes = new_nodes;
  return *this;
}

SearchConfig& SearchConfig::set_movetime(std::chrono::milliseconds new_time) {
  movetime = new_time;
  return *this;
}

SearchConfig& SearchConfig::set_infinite(bool new_infinite) {
  infinite = new_infinite;
  return *this;
}

}  // namespace engine::uci