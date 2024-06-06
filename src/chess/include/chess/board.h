#pragma once

#include <string_view>
#include <vector>

#include "bitboard.h"
#include "move_container.h"
#include "player.h"

class Board {
public:
  Board(Player white, Player black, u64 en_passant_bit, bool is_white_turn);

  // Starting board of a chess game.
  static Board initial();

  // Construct a board from Extended Position Description.
  // https://www.chessprogramming.org/Extended_Position_Description
  static Board from_epd(std::string_view epd);

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

private:
  Player white;
  Player black;
  u64 en_passant_bit;  // The square that is currently capturable by en passant (if any).
  bool is_white_turn;
};