#pragma once

#include <atomic>
#include <istream>
#include <memory>
#include <mutex>
#include <ostream>
#include <string_view>
#include <thread>
#include <vector>

#include "chess/board.h"
#include "chess_engine/engine.h"
#include "chess_engine/uci.h"
#include "outputs/output.h"
#include "uci_io.h"

class EngineCli {
public:
  explicit EngineCli(std::istream& input_stream, std::ostream& output_stream);

  void start();

  [[nodiscard]] std::string_view get_author() const;

  [[nodiscard]] std::string_view get_name() const;

  // Prepare for a new game by clearing cached data about the current game (if any).
  void new_game();

  // Set the debug mode.
  void set_debug(bool new_debug_mode);

  // Update the position.
  void set_position(chess::Board new_position, std::vector<chess::Move> new_moves);

  // Write a response to the output stream.
  void write(const Output& output);

  // Start searching the current position.
  void go(engine::uci::SearchConfig config);

  // Stops the current search, if any.
  void stop();

  // Wait until the current search is done. Returns immediately if there's no ongoing search.
  void wait() const;

  // Quit this application.
  void quit();

private:
  UciIO uci_io;
  std::mutex output_mutex;  // Controls access to `uci_io.write`
  chess::Board position;
  std::vector<chess::Move> moves;
  bool debug_mode;
  bool done;

  // A threadsafe class to manage the ongoing search.
  class OngoingSearch {
  public:
    explicit OngoingSearch();

    // Returns true if the search starts successfully.
    // Else returns false (i.e. another search is already ongoing).
    bool go(chess::Board position, std::vector<chess::Move> moves, engine::uci::SearchConfig config,
            EngineCli& engine_cli);

    // Stops the current search, if any.
    void stop();

    // Wait until the current search is done. Returns immediately if there's no ongoing search.
    void wait() const;

    // Resets the engine state.
    void reset();

    ~OngoingSearch();

  private:
    Engine engine;
    std::thread thread;
    std::atomic<bool> ongoing;
    std::shared_ptr<engine::Search> search_control;
  };
  OngoingSearch ongoing_search;
};