#include "search.h"

#include "search_impl.h"

void engine::Search::stop() { impl->stop(); }

void engine::Search::wait_for_done() const { impl->wait_for_done(); }

chess::Move engine::Search::get_move() const { return impl->get_move(); }

engine::Search::DebugInfo engine::Search::get_debug_info() const { return impl->get_debug_info(); }

engine::Search::Search(std::unique_ptr<Search::Impl> impl) : impl{std::move(impl)} {}

engine::Search::~Search() = default;