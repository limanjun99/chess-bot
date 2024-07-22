#include "engine.h"

#include "chess/board.h"
#include "chess/move.h"
#include "engine_impl.h"
#include "search_impl.h"

Engine::Engine(chess::Board board, std::span<chess::Move const> moves)
    : impl{std::make_unique<Engine::Impl>(std::move(board), moves)} {}

void Engine::set_position(chess::Board board, std::span<chess::Move const> moves) {
  impl->set_position(std::move(board), moves);
}

void Engine::reset() { impl->reset(); }

void Engine::apply_move(const chess::Move& move) { impl->apply_move(move); }

std::shared_ptr<engine::Search> Engine::search(engine::uci::SearchConfig config) {
  return impl->search(std::move(config));
}

std::pair<chess::Move, engine::Search::DebugInfo> Engine::search_sync(engine::uci::SearchConfig config) {
  auto result{search(std::move(config))};
  return std::make_pair(result->get_move(), result->get_debug_info());
}

Engine::~Engine() = default;