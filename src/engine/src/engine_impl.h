#pragma once

#include <memory>

#include "chess/board.h"
#include "chess/stack_repetition_tracker.h"
#include "engine.h"
#include "heuristics.h"

class Engine::Impl {
public:
  explicit Impl(chess::Board board = chess::Board::initial(), std::span<chess::Move const> moves = {});

  // Set the board position based on applying the `moves` to the `position`.
  void set_position(chess::Board position, std::span<chess::Move const> moves = {});

  // Clears all data about the current game.
  void reset();

  // Apply the given move to the board.
  void apply_move(const chess::Move& move);

  // Performs a search with configurations based on the uci go command.
  // The search can be interacted with through the returned Search object.
  std::shared_ptr<engine::Search> search(engine::uci::SearchConfig config);

private:
  chess::Board current_position;
  chess::StackRepetitionTracker repetition_tracker;
  std::shared_ptr<Heuristics> heuristics;
};