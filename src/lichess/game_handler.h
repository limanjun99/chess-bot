#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>

#include "chess/board.h"
#include "chess_engine/engine.h"
#include "config.h"
#include "lichess.h"

using json = nlohmann::json;

// Handles a single Lichess game.
class GameHandler {
public:
  GameHandler(const Config& config, const Lichess& lichess, const std::string& game_id);

  // Start listening and responding to incoming game events from Lichess API.
  void listen();

private:
  const Config& config;
  const Lichess& lichess;
  const std::string& game_id;
  bool is_white;                    // Whether the bot is playing as white.
  std::chrono::milliseconds wtime;  // Amount of time left for white player.
  std::chrono::milliseconds winc;   // Amount of increment per move for white player.
  std::chrono::milliseconds btime;  // Amount of time left for black player.
  std::chrono::milliseconds binc;   // Amount of increment per move for black player.
  int ply_count;                    // Number of plys that we received from Lichess API so far.
  chess::Board board;               // Current board of the game.
  Engine engine;

  // Find the move to make in the given position.
  std::pair<chess::Move, engine::Search::DebugInfo> choose_move();

  // Refer to Lichess API for shape of `game` object (type "gameFull").
  // Called once at the start of the game. Returns true if we should continue playing.
  bool handle_game_initialization(const json& game);

  // Refer to Lichess API for shape of `state` object (type "gameState").
  // Called once per move by either player. Returns true if we should continue playing.
  bool handle_game_update(const json& state);

  // Refer to Lichess API for shape of `state` object (type "gameState").
  // Called once when the game ends / was aborted.
  void handle_game_end(const json& state);

  // Given a string of moves in UCI format, update the current board state to match it.
  void update_board(const std::string& moves);

  friend class Lichess;
};