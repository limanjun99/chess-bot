#include "lichess.h"

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>

#include "chess_engine/engine.h"
#include "logger.h"

using json = nlohmann::json;

namespace api {
std::string base{"https://lichess.org/api"};
std::string stream_incoming_events{base + "/stream/event"};
std::string accept_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/accept";
}
std::string decline_challenge(const std::string& challenge_id) {
  return base + "/challenge/" + challenge_id + "/decline";
}
std::string stream_game(const std::string& game_id) { return base + "/bot/game/stream/" + game_id; }
std::string make_move(const std::string& game_id, const std::string& move) {
  return base + "/bot/game/" + game_id + "/move/" + move;
}
};  // namespace api

Lichess::Lichess(const Config& config) : config{config} {}

void Lichess::listen() {
  auto callback = [this](std::string data, intptr_t) { return handle_incoming_event(std::move(data)); };
  cpr::Get(cpr::Url{api::stream_incoming_events}, cpr::Bearer{config.get_lichess_token()},
           cpr::WriteCallback{callback});
}

bool Lichess::handle_challenge(const std::string& challenge_id, bool accept) {
  std::string url = accept ? api::accept_challenge(challenge_id) : api::decline_challenge(challenge_id);
  cpr::Response res = cpr::Post(cpr::Url{std::move(url)}, cpr::Bearer{config.get_lichess_token()});
  return res.status_code == 200;
}

void Lichess::handle_game(const std::string& game_id) {
  GameHandler game_handler{config, game_id};
  game_handler.listen();
}

bool Lichess::handle_incoming_event(std::string data) {
  while (!data.empty() && data.back() == '\n') data.pop_back();
  if (data.empty()) return true;
  json event = json::parse(data);

  if (event["type"] == "challenge") {
    auto challenge = event["challenge"];
    // Ignore all requests sent out by the bot.
    if (challenge["challenger"]["id"] == config.get_lichess_bot_name()) return true;
    // Decline all rated challenges.
    if (challenge["rated"]) {
      handle_challenge(challenge["id"], false);
      Logger::info() << "Declined challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
      return true;
    }
    // Accept all other challenges.
    handle_challenge(challenge["id"], true);
    Logger::info() << "Accepted challenge " << challenge["id"] << " from " << challenge["challenger"]["id"] << "\n";
  } else if (event["type"] == "gameStart") {
    auto game = event["game"];
    handle_game(game["gameId"]);
  }
  return true;
}

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
  cpr::Get(cpr::Url{api::stream_game(game_id)}, cpr::Bearer{config.get_lichess_token()}, cpr::WriteCallback{callback});
}

Engine::MoveInfo GameHandler::choose_move(const Board& board) {
  return engine.choose_move(board, std::chrono::seconds{2});
}

bool GameHandler::handle_game_event(const std::string& game_id, std::string_view data) {
  if (data.empty()) return true;
  json event = json::parse(data);

  std::optional<Board> board_opt = std::nullopt;
  if (event["type"] == "gameFull") {
    // Initialise game data.
    is_white = event["white"]["id"] == config.get_lichess_bot_name();
    board_opt = std::optional(initialise_board(event["state"]["moves"]));
  } else if (event["type"] == "gameState") {
    if (event["status"] != "started") {
      return false;  // Game ended.
    }
    board_opt = std::optional(initialise_board(event["moves"]));
  }
  if (!board_opt.has_value()) return true;  // Not an event caused by a move.
  auto board = *board_opt;
  if (board.is_white_to_move() != is_white) return true;  // Not my turn.

  engine.add_position(board);
  Engine::MoveInfo move_info = choose_move(board);
  send_move(move_info.move);
  engine.add_position(board.apply_move(move_info.move));
  Logger::info() << "Found move " << move_info.move.to_uci() << " for game " << game_id << " in "
                 << move_info.time_spent.count() << "ms (depth " << move_info.search_depth << " reached, "
                 << move_info.normal_node_count / 1000 << "k nodes, " << move_info.quiescence_node_count / 1000
                 << "k quiescent nodes)\n";
  return true;
}

Board GameHandler::initialise_board(const std::string& moves) {
  Board board = Board::initial();
  std::istringstream moves_stream{moves};
  std::string move;
  while (moves_stream >> move) board = board.apply_uci_move(move);
  return board;
}

void GameHandler::send_move(const Move& move) {
  std::string uci{move.to_uci()};
  cpr::Response res = cpr::Post(cpr::Url{api::make_move(game_id, uci)}, cpr::Bearer{config.get_lichess_token()});
}