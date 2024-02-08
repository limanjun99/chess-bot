#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

#include "chess/board.h"
#include "chess_engine/engine.h"
#include "config.h"

using json = nlohmann::json;

// Handles a single Lichess game.
class GameHandler {
public:
  GameHandler(const Config& config, const std::string& game_id);

  // Start listening and responding to incoming game events from Lichess API.
  void listen();

private:
  const Config& config;
  const std::string& game_id;
  bool is_white;                  // Whether the bot is playing as white.
  int increment;                  // Amount of increment (in milliseconds) per move.
  int time_left;                  // Amount of time left (in milliseconds) I have.
  Board board{Board::initial()};  // Current board of the game.
  Engine engine{};

  // Find the move to make in the given position.
  Engine::MoveInfo choose_move(const Board& board);

  // Handle a game event from an ongoing game. Returns false if the game has ended.
  bool handle_game_event(const std::string& game_id, std::string_view data);

  // The first event received from Lichess API contains full game data, which will be used to initialise this object.
  void initialise(const json& event);

  // After each event, update the state of the board and time.
  void update_state(const json& state);

  // Send the given move to the server.
  void send_move(const Move& move);
};