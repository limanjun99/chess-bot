#include "engine_cli.h"

#include <functional>

#include "chess_engine/engine.h"
#include "outputs/best_move_output.h"
#include "outputs/error_output.h"

EngineCli::EngineCli(std::istream& input_stream, std::ostream& output_stream)
    : uci_io{input_stream, output_stream},
      output_mutex{},
      position{chess::Board::initial()},
      debug_mode{false},
      done{false} {}

void EngineCli::start() {
  while (!done) {
    const auto command{uci_io.read()};
    if (!command) {
      bool done{false};
      switch (command.error().error_type) {
        case UciIO::ReadError::EndOfFile:
          done = true;
          break;
        case UciIO::ReadError::InvalidCommand:
          uci_io.write(ErrorOutput{std::move(command.error().error_message)});
          break;
      }
      if (done) break;
      else continue;
    }

    (*command)->execute(*this);
  }
}

std::string_view EngineCli::get_author() const { return "placeholderAuthor"; }

std::string_view EngineCli::get_name() const { return "placeholderName"; }

void EngineCli::new_game() {
  //! TODO: Implement.
}

void EngineCli::set_debug(bool new_debug_mode) { debug_mode = new_debug_mode; }

void EngineCli::set_position(chess::Board new_position) { position = std::move(new_position); }

void EngineCli::write(const Output& output) {
  std::scoped_lock output_lock{output_mutex};
  uci_io.write(output);
}

void EngineCli::go(engine::uci::SearchConfig config) {
  const bool started{ongoing_search.go(position, std::move(config), *this)};
  if (!started) {
    //! TODO (low priority): This write could occur too late, for example if the go command
    //! finishes and outputs before this write is called.
    uci_io.write(ErrorOutput{"Cannot use go command while a previous go command is in progress."});
  }
}

void EngineCli::stop() { ongoing_search.stop(); }

void EngineCli::wait() const { ongoing_search.wait(); }

void EngineCli::quit() { done = true; }

EngineCli::OngoingSearch::OngoingSearch() : engine{}, thread{}, ongoing{false}, search_control{nullptr} {}

bool EngineCli::OngoingSearch::go(chess::Board position, engine::uci::SearchConfig config, EngineCli& engine_cli) {
  bool currently_ongoing{false};
  ongoing.compare_exchange_strong(currently_ongoing, true, std::memory_order_acq_rel);
  if (currently_ongoing) return false;

  if (thread.joinable()) thread.join();
  const auto handle_search{[this](chess::Board position, engine::uci::SearchConfig config, EngineCli& engine_cli) {
    engine.set_position(std::move(position));
    search_control = engine.cancellable_search(std::move(config));
    const chess::Move best_move{search_control->wait_and_get_move()};
    const BestMoveOutput output{best_move};
    engine_cli.write(output);
    ongoing.store(false, std::memory_order_release);
    ongoing.notify_all();
  }};

  thread = std::thread{handle_search, std::move(position), std::move(config), std::ref(engine_cli)};
  return true;
}

void EngineCli::OngoingSearch::wait() const { ongoing.wait(true, std::memory_order_acquire); }

EngineCli::OngoingSearch::~OngoingSearch() {
  if (thread.joinable()) thread.join();
}

void EngineCli::OngoingSearch::stop() { search_control->stop(); }