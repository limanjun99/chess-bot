#include "game_handler.h"

#include <algorithm>

#include "chess/uci.h"
#include "chess_engine/uci.h"
#include "lichess.h"
#include "logger.h"

GameHandler::GameHandler(const Config& config, const Lichess& lichess, const std::string& game_id)
    : config{config},
      lichess{lichess},
      game_id{game_id},
      is_white{true},
      wtime{},
      winc{},
      btime{},
      binc{},
      ply_count{0},
      board{chess::Board::initial()},
      engine{} {}

void GameHandler::listen() {
  Logger::get().format_info("Handling game {}", game_id);
  lichess.handle_game(game_id, *this);
}

std::pair<chess::Move, engine::Search::DebugInfo> GameHandler::choose_move() {
  engine::uci::SearchConfig search_config{};
  search_config.set_wtime(wtime);
  search_config.set_winc(winc);
  search_config.set_btime(btime);
  search_config.set_binc(binc);

  //! TODO: Find a better way to account for network latency.
  std::chrono::milliseconds latency_compensation{1000};
  auto my_time{is_white ? wtime : btime};
  auto my_inc{is_white ? winc : binc};
  std::chrono::milliseconds inc_compensation{std::min(my_inc, latency_compensation)};
  my_inc -= inc_compensation;
  latency_compensation -= inc_compensation;
  my_time = std::max(std::chrono::milliseconds{0}, my_time - latency_compensation);
  if (is_white) {
    search_config.set_wtime(my_time);
    search_config.set_winc(my_inc);
  } else {
    search_config.set_btime(my_time);
    search_config.set_binc(my_inc);
  }

  if (ply_count <= 1) {
    // Our clock only starts ticking after the first move.
    // Hence, we spend a fixed time of 1 second on the first move,
    // as thinking longer probably does not help.
    search_config.set_movetime(std::chrono::seconds{1});
  }

  return engine.search_sync(std::move(search_config));
}

bool GameHandler::handle_game_initialization(const json& game) {
  is_white = game["white"].value("id", "") == config.get_lichess_bot_name();
  board = chess::Board::initial();
  return handle_game_update(game["state"]);
}

bool GameHandler::handle_game_update(const json& state) {
  // Update the board.
  update_board(state["moves"]);
  // Update the timers.
  wtime = std::chrono::milliseconds{state["wtime"]};
  winc = std::chrono::milliseconds{state["winc"]};
  btime = std::chrono::milliseconds{state["btime"]};
  binc = std::chrono::milliseconds{state["binc"]};

  if (board.is_white_to_move() != is_white) return true;  // Not my turn.

  const auto [move, debug] = choose_move();
  lichess.send_move(game_id, move.to_uci());

  Logger::get().format_info(
      "Found move {} for game {} in {}ms (depth {} reached, {}k nodes, {}k quiescent nodes, {}/{}k TT, {}/{}k NM, "
      "{}/{}k QDP, {} eval)",
      move.to_algebraic(), game_id, debug.time_spent.count(), debug.search_depth, debug.normal_node_count / 1000,
      debug.quiescence_node_count / 1000, debug.transposition_table_success / 1000,
      debug.transposition_table_total / 1000, debug.null_move_success / 1000, debug.null_move_total / 1000,
      debug.q_delta_pruning_success / 1000, debug.q_delta_pruning_total / 1000, debug.evaluation);
  return true;
}

void GameHandler::handle_game_end(const json& state) {
  Logger::get().format_info("Game ended with status: {}", state["status"].dump());
}

void GameHandler::update_board(const std::string& moves) {
  std::istringstream moves_stream{moves};
  std::string uci_move;
  for (int i = 0; moves_stream >> uci_move; i++) {
    if (i < ply_count) continue;
    ply_count++;
    const chess::Move move{chess::uci::move(uci_move, board)};
    engine.apply_move(move);
    board = board.apply_move(move);
  }
}