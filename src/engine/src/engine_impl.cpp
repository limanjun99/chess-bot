#include "engine_impl.h"

#include "search_impl.h"

Engine::Impl::Impl(chess::Board position, std::span<chess::Move const> moves)
    : current_position{chess::Board::initial()}, repetition_tracker{}, heuristics{std::make_shared<Heuristics>()} {
  set_position(std::move(position), moves);
}

void Engine::Impl::set_position(chess::Board position, std::span<chess::Move const> moves) {
  repetition_tracker = chess::StackRepetitionTracker{};
  current_position = position;
  repetition_tracker.push(current_position);
  for (const auto& move : moves) {
    current_position = current_position.apply_move(move);
    repetition_tracker.push(current_position, move);
  }
}

void Engine::Impl::reset() {
  // Reset all cached data.
  heuristics = std::make_shared<Heuristics>();
  set_position(chess::Board::initial());
}

void Engine::Impl::apply_move(const chess::Move& move) {
  current_position = current_position.apply_move(move);
  repetition_tracker.push(current_position, move);
}

std::shared_ptr<engine::Search> Engine::Impl::search(engine::uci::SearchConfig config) {
  auto search_impl{
      std::make_unique<engine::Search::Impl>(current_position, repetition_tracker, heuristics, std::move(config))};
  return engine::Search::Impl::to_search(std::move(search_impl));
}