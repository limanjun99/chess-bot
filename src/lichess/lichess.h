#pragma once

#include "chess/board.h"
#include "config.h"

class Lichess {
public:
  Lichess(const Config& config);

  // Start listening and responding to incoming events from Lichess API.
  void listen();

private:
  const Config& config;

  // Either accept or decline the challenge with given `challenge_id`.
  // Returns false if challenge was not found.
  bool handle_challenge(const std::string& challenge_id, bool accept);

  // Handle the ongoing game with given `game_id`.
  void handle_game(const std::string& game_id);

  // Handle an incoming event. Returns true if handled successfully.
  bool handle_incoming_event(std::string data);
};

class GameHandler {
public:
  GameHandler(const Config& config, const std::string& game_id);

  // Start listening and responding to incoming game events from Lichess API.
  void listen();

private:
  const Config& config;
  const std::string& game_id;
  bool is_white;  // Whether the bot is playing as white.

  // Find the move to make in the given position.
  Move find_move(const Board& board);

  // Handle a game event from an ongoing game.
  void handle_game_event(const std::string& game_id, std::string data);

  // Initialise a board after applying all the moves to a starting position.
  // Moves are in UCI format.
  Board initialise_board(const std::string& moves);

  // Send the given move to the server.
  void send_move(const Move& move);
};