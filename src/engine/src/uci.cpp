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

SearchConfig::Builder::Builder() : config{} {}

void SearchConfig::Builder::add_search_moves(chess::Move move) { config.search_moves.push_back(move); }

void SearchConfig::Builder::set_wtime(std::chrono::milliseconds time) { config.wtime = time; }

void SearchConfig::Builder::set_winc(std::chrono::milliseconds increment) { config.winc = increment; }

void SearchConfig::Builder::set_btime(std::chrono::milliseconds time) { config.btime = time; }

void SearchConfig::Builder::set_binc(std::chrono::milliseconds increment) { config.binc = increment; }

void SearchConfig::Builder::set_depth(int depth) { config.depth = depth; }

void SearchConfig::Builder::set_nodes(int64_t nodes) { config.nodes = nodes; }

void SearchConfig::Builder::set_movetime(std::chrono::milliseconds time) { config.movetime = time; }

void SearchConfig::Builder::set_infinite(bool infinite) { config.infinite = infinite; }

SearchConfig SearchConfig::Builder::build() { return std::move(config); }

}  // namespace engine::uci