#pragma once

#include <string_view>

#include "bitboard.h"
#include "moveset.h"
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
  Board apply_move(Piece piece, u64 from, u64 to) const;

  // Returns a new board that is the result of applying the given promotion to the given piece.
  Board apply_promotion(u64 from, u64 to, Piece piece) const;

  // Returns a new board that is the result of applying the given move in UCI notation (assumed to be valid).
  Board apply_uci_move(std::string_view uci_move);

  // Returns a reference to the current player whose turn it is.
  const Player& cur_player() const;
  Player& cur_player();

  // Generate a MoveSet representation of all legal moves.
  MoveSet generate_moves() const;

  // Returns true if the current player is in check.
  bool is_in_check() const;

  // Check if the given square is under attack by the opponent.
  bool is_under_attack(u64 square) const;

  // Return whether it is white to move.
  bool is_white_to_move() const;

  // Returns true if the other player is in check.
  bool moved_into_check() const;

  // Returns a reference to the opponent of the current player.
  const Player& opp_player() const;
  Player& opp_player();

  // Returns a 8x8 newline delimited string represenation of the board.
  std::string to_string() const;

private:
  Player white;
  Player black;
  u64 en_passant_bit;  // The pawn that is currently capturable by en passant (if any).
  bool is_white_turn;

  // Generate legal bishop moves and add to move_set.
  void generate_bishop_moves(MoveSet& move_set) const;
  // Generate legal king moves and add to move_set.
  void generate_king_moves(MoveSet& move_set) const;
  // Generate legal knight moves and add to move_set.
  void generate_knight_moves(MoveSet& move_set) const;
  // Generate legal pawn moves and add to move_set.
  void generate_pawn_moves(MoveSet& move_set) const;
  // Generate legal queen moves and add to move_set.
  void generate_queen_moves(MoveSet& move_set) const;
  // Generate legal rook moves and add to move_set.
  void generate_rook_moves(MoveSet& move_set) const;
};