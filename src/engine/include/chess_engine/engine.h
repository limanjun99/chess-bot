#pragma once

#include <memory>
#include <span>
#include <utility>

#include "chess/board.h"
#include "chess/move.h"
#include "search.h"
#include "uci.h"

class Engine {
public:
  explicit Engine(chess::Board position = chess::Board::initial(), std::span<chess::Move const> moves = {});

  // Set the board position based on applying the `moves` to the `position`.
  void set_position(chess::Board position, std::span<chess::Move const> moves = {});

  // Clears all data about the current game.
  void reset();

  // Apply the given move to the board.
  void apply_move(const chess::Move& move);

  // Performs a search with configurations based on the uci go command.
  // The search can be interacted with through the returned Search object.
  [[nodiscard]] std::shared_ptr<engine::Search> search(engine::uci::SearchConfig config);

  // Returns the best move and debug info, after running a search with the given configuration.
  // Essentially a wrapper for {search(config).get_move(), search(config).get_debug_info()}.
  [[nodiscard]] std::pair<chess::Move, engine::Search::DebugInfo> search_sync(engine::uci::SearchConfig config);

  Engine(const Engine&) = delete;
  Engine(Engine&&) = default;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = default;
  ~Engine();

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
