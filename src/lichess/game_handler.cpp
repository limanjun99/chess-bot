#include "game_handler.h"

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include "lichess.h"
#include "logger.h"

using json = nlohmann::json;

GameHandler::GameHandler(const Config& config, const std::string& game_id) : config{config}, game_id{game_id} {}

void GameHandler::listen() {
  Logger::info() << "Handling game " << game_id << "\n";
  auto callback = [this](std::string data, intptr_t) {
    bool ongoing = true;
    size_t start = 0;
    size_t end = 0;
    while (end != std::string::npos) {
      while (start < data.size() && data[start] == '\n') start++;
      if (start >= data.size()) break;
      end = data.find("\n", start + 1);
      if (end == std::string::npos) end = data.size();
      ongoing &= handle_game_event(game_id, std::string_view{data}.substr(start, end - start));
      start = end + 1;
    }
    return ongoing;
  };
  cpr::Get(cpr::Url{lichess_api::stream_game(game_id)}, cpr::Bearer{config.get_lichess_token()},
           cpr::WriteCallback{callback});
}

Engine::MoveInfo GameHandler::choose_move(const Board& board) {
  // The time spent by the engine is equal to (time_left / 60 + increment),
  // and it will be bounded to within min(time_left / 2, 100ms) ~ 10s.
  // Note that for now we -800ms to account for network latency.
  //! TODO: Find a better way to account for network latency.
  int engine_time = time_left / 60 + increment - 800;
  engine_time = std::min(engine_time, time_left / 2);
  engine_time = std::min(engine_time, 10'000);
  engine_time = std::max(engine_time, 100);
  return engine.choose_move(board, std::chrono::milliseconds{engine_time});
}

bool GameHandler::handle_game_event(const std::string& game_id, std::string_view data) {
  if (data.empty()) return true;
  json event = json::parse(data);

  if (event["type"] == "gameFull") {
    // Initialise game data.
    initialise(event);
  } else if (event["type"] == "gameState") {
    if (event["status"] != "started") {
      return false;  // Game ended.
    }
    update_state(event);
  } else {
    // Ignore other types of event like chat.
    return true;
  }
  if (board.is_white_to_move() != is_white) return true;  // Not my turn.

  engine.add_position(board);
  Engine::MoveInfo move_info = choose_move(board);
  send_move(move_info.move);
  engine.add_position(board.apply_move(move_info.move));
  Logger::info() << "Found move " << move_info.move.to_uci() << " for game " << game_id << " in "
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

void GameHandler::initialise(const json& event) {
  is_white = event["white"]["id"] == config.get_lichess_bot_name();
  update_state(event["state"]);
}

void GameHandler::update_state(const json& state) {
  // Reset the board state, and apply all moves to get the current board.
  const std::string& moves = state["moves"];
  board = Board::initial();
  std::istringstream moves_stream{moves};
  std::string move;
  while (moves_stream >> move) board = board.apply_uci_move(move);

  // Update our timers.
  if (is_white) {
    increment = state["winc"];
    time_left = state["wtime"];
  } else {
    increment = state["binc"];
    time_left = state["btime"];
  }
}

void GameHandler::send_move(const Move& move) {
  std::string uci{move.to_uci()};
  cpr::Response res =
      cpr::Post(cpr::Url{lichess_api::make_move(game_id, uci)}, cpr::Bearer{config.get_lichess_token()});
}