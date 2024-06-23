#include "game_handler.h"

#include <cpr/cpr.h>

#include "lichess.h"
#include "logger.h"

GameHandler::GameHandler(const Config& config, const Lichess& lichess, const std::string& game_id)
    : config{config}, lichess{lichess}, game_id{game_id} {}

void GameHandler::listen() {
  Logger::info() << "Handling game " << game_id << "\n";
  lichess.handle_game(game_id, *this);
}

Engine::MoveInfo GameHandler::choose_move(const chess::Board& board) {
  // The time spent by the engine is equal to (time_left / 60 + increment),
  // and it will be bounded to within min(time_left / 2, 100ms) ~ 10s.
  // Note that for now we -800ms to account for network latency.
  //! TODO: Find a better way to account for network latency.
  int engine_time = time_left / 60 + increment - 800;
  engine_time = std::min(engine_time, time_left / 2);
  engine_time = std::min(engine_time, 10'000);
  engine_time = std::max(engine_time, 100);
  if (ply_count <= 1) {
    // Spend only 1 second on the first move, as it is not important,
    // and to prevent the game being aborted from us idling too long.
    engine_time = 1000;
  }
  return engine.choose_move(board, std::chrono::milliseconds{engine_time});
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

  Engine::MoveInfo move_info = choose_move(board);
  lichess.send_move(game_id, move_info.move.to_uci());
  Logger::info() << "Found move " << move_info.move.to_algebraic() << " for game " << game_id << " in "
                 << move_info.time_spent.count() << "ms (depth " << move_info.search_depth << " reached, "
                 << move_info.debug.normal_node_count / 1000 << "k nodes, "
                 << move_info.debug.quiescence_node_count / 1000 << "k quiescent nodes, "
                 << move_info.debug.transposition_table_success / 1000 << "/"
                 << move_info.debug.transposition_table_total / 1000 << "k TT, "
                 << move_info.debug.null_move_success / 1000 << "/" << move_info.debug.null_move_total / 1000
                 << "k NM, " << move_info.debug.q_delta_pruning_success / 1000 << "/"
                 << move_info.debug.q_delta_pruning_total / 1000 << "k QDP, " << move_info.debug.evaluation
                 << " eval)\n";
  Logger::flush();
  return true;
}

void GameHandler::handle_game_end(const json& state) {
  Logger::info() << "Game ended with status: " << state["status"] << "\n";
  Logger::flush();
}

void GameHandler::update_board(const std::string& moves) {
  std::istringstream moves_stream{moves};
  std::string move;
  for (int i = 0; moves_stream >> move; i++) {
    if (i < ply_count) continue;
    ply_count++;
    board = board.apply_uci_move(move);
    engine.add_position(board);
  }
}