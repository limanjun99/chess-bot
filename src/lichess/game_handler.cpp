#include "game_handler.h"

#include <cpr/cpr.h>

#include "chess/uci.h"
#include "chess_engine/uci.h"
#include "lichess.h"
#include "logger.h"

GameHandler::GameHandler(const Config& config, const Lichess& lichess, const std::string& game_id)
    : config{config}, lichess{lichess}, game_id{game_id} {}

void GameHandler::listen() {
  Logger::info() << "Handling game " << game_id << "\n";
  lichess.handle_game(game_id, *this);
}

std::pair<chess::Move, engine::Search::DebugInfo> GameHandler::choose_move() {
  // The time spent by the engine is equal to (time_left / 120 + increment),
  // and it will be bounded to within min(time_left / 2, 100ms) ~ 10s.
  // Note that for now we -1200ms to account for network latency.
  //! TODO: Find a better way to account for network latency.
  int engine_time = time_left / 120 + increment - 1200;
  engine_time = std::min(engine_time, time_left / 2);
  engine_time = std::min(engine_time, 10'000);
  engine_time = std::max(engine_time, 100);
  if (ply_count <= 1) {
    // Spend only 1 second on the first move, as it is not important,
    // and to prevent the game being aborted from us idling too long.
    engine_time = 1000;
  }
  return engine.search_sync(engine::uci::SearchConfig::from_movetime(std::chrono::milliseconds{engine_time}));
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
  increment = state[is_white ? "winc" : "binc"];
  time_left = state[is_white ? "wtime" : "btime"];

  if (board.is_white_to_move() != is_white) return true;  // Not my turn.

  const auto [move, debug] = choose_move();
  lichess.send_move(game_id, move.to_uci());
  Logger::info() << "Found move " << move.to_algebraic() << " for game " << game_id << " in "
                 << debug.time_spent.count() << "ms (depth " << debug.search_depth << " reached, "
                 << debug.normal_node_count / 1000 << "k nodes, " << debug.quiescence_node_count / 1000
                 << "k quiescent nodes, " << debug.transposition_table_success / 1000 << "/"
                 << debug.transposition_table_total / 1000 << "k TT, " << debug.null_move_success / 1000 << "/"
                 << debug.null_move_total / 1000 << "k NM, " << debug.q_delta_pruning_success / 1000 << "/"
                 << debug.q_delta_pruning_total / 1000 << "k QDP, " << debug.evaluation << " eval)\n";
  Logger::flush();
  return true;
}

void GameHandler::handle_game_end(const json& state) {
  Logger::info() << "Game ended with status: " << state["status"] << "\n";
  Logger::flush();
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