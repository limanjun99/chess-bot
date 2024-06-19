#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "bitboard.h"
#include "move_container.h"
#include "player.h"

class Board {
public:
  Board(Player white, Player black, u64 en_passant_bit, bool is_white_turn, int16_t halfmove_clock = 0);

  // Starting board of a chess game.
  static Board initial();

  // Construct a board from FEN.
  // https://www.chessprogramming.org/Forsyth-Edwards_Notation
  static Board from_fen(std::string_view fen);

  // Returns a new board that is the result of applying the given move.
  Board apply_move(const Move& move) const;

  // Returns a new board that is the result of applying the given move in UCI notation (assumed to be valid).
  Board apply_uci_move(std::string_view uci_move);

  // Returns a reference to the current player whose turn it is.
  const Player& cur_player() const;
  Player& cur_player();

  // Generate a list of all legal captures and promotions.
  MoveContainer generate_quiescence_moves() const;

  // Generate a list of all legal captures, checks and promotions.
  MoveContainer generate_quiescence_moves_and_checks() const;

  // Generate a list of all legal moves.
  MoveContainer generate_moves() const;

  // Returns true if the current player still has any move to make.
  bool has_moves() const;

  // Check if the given move is a check. Note that this function is not optimized.
  bool is_a_check(const Move& move) const;

  // Returns true if the current player is in check.
  bool is_in_check() const;

  // Check if the given square is under attack by the opponent.
  bool is_under_attack(u64 square) const;

  // Return whether it is white to move.
  inline bool is_white_to_move() const { return is_white_turn; }

  // Returns a reference to the opponent of the current player.
  const Player& opp_player() const;
  Player& opp_player();

  // Return a new board, where the current player skipped their turn.
  Board skip_turn() const;

  // Returns a reference to the white player.
  const Player& get_white() const;

  // Returns a reference to the black player.
  const Player& get_black() const;

  // Returns the bitboard for the en passant bit.
  u64 get_en_passant() const;

  // Returns a 8x8 newline delimited string represenation of the board.
  std::string to_string() const;

  // Returns the current turn's player color.
  Color get_color() const;

  // Returns the Zobrist hash of this board.
  u64 get_hash() const;

  // Returns true if the game has ended.
  bool is_game_over() const;

  // Returns std::nullopt if the game has not ended.
  // Else returns the score (-1 if black won, 0 if draw, 1 if white won).
  std::optional<int32_t> get_score() const;

  // Returns std::nullopt if the game has not ended.
  // Else returns the score of the current player (-1 if current player won, 0 if draw, else 1).
  std::optional<int32_t> get_player_score() const;

private:
  Player white;
  Player black;
  // The square that is currently capturable by en passant (if any).
  u64 en_passant_bit;
  // Keeps track of board positions (using their Zobrist hash) since the last unrepeatable move.
  //! TODO: Copying all positions for each new Board has terrible performance.
  std::vector<u64> tracked_positions;
  // Keeps track of number of plies without capture / pawn pushes (for fifty move rule).
  int16_t halfmove_clock;
  bool is_white_turn;

  // Returns true if the position is a draw based on fifty move rule / threefold repetition.
  bool is_stagnant_draw() const;
};