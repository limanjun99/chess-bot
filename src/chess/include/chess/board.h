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

  // Returns a new board that is the result of applying the given move.
  Board apply_move(Piece piece, u64 from, u64 to) const;

  // Returns a new board that is the result of applying the given promotion to the given piece.
  Board apply_promotion(u64 from, u64 to, Piece piece) const;

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

  // Check if the given move is a capture.
  bool is_a_capture(const Move& move) const;

  // Check if the given move is a check. Note that this function is not optimized.
  bool is_a_check(const Move& move) const;

  // Returns true if the current player is in check.
  bool is_in_check() const;

  // Check if the given square is under attack by the opponent.
  bool is_under_attack(u64 square) const;

  // Return whether it is white to move.
  inline bool is_white_to_move() const { return is_white_turn; }

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
  u64 en_passant_bit;  // The square that is currently capturable by en passant (if any).
  u64 cur_occupied;
  u64 opp_occupied;
  bool is_white_turn;

  // Either add the pawn move, or add all 4 possible promotions if it is a promotion.
  void add_pawn_moves(MoveContainer& moves, u64 from, u64 to) const;
  // Returns a bitboard of opponent pieces that attack the king.
  u64 get_king_attackers() const;
  // Returns a bitboard of my pieces that are pinned (i.e. removing them will open up the king to an attacker).
  u64 get_pinned_pieces() const;

  enum class MoveType { All, CapturesAndPromotionsOnly, CapturesChecksAndPromotionsOnly };
  // Generate legal bishop moves (and queen moves with bishop movement) given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_bishoplike_moves(MoveContainer& moves, u64 pinned_pieces) const;
  // Generate legal king moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_king_moves(MoveContainer& moves) const;
  // Generate legal knight moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_knight_moves(MoveContainer& moves, u64 pinned_pieces) const;
  // Generate legal pawn moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_pawn_moves(MoveContainer& moves, u64 pinned_pieces) const;
  // Generate legal rook moves (and queen moves with rook movement) given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_rooklike_moves(MoveContainer& moves, u64 pinned_pieces) const;

  // Generate legal king moves that escape the single-check.
  void generate_king_single_check_evasions(MoveContainer& moves, u64 attacker) const;
  // Generate legal king moves that escape the double-check.
  template <MoveType MT>
  void generate_king_double_check_evasions(MoveContainer& moves) const;

  // TODO: The below methods `has_..._moves` are created solely for quickly checking if there are any more moves in a
  // position (without generating all moves). This is extremely ugly, and there is significant overlap with
  // `generate_..._moves`. Find a better way to do it.

  // Check if there are legal bishop moves (and queen moves with bishop movement) given that the king is not in check.
  bool has_unchecked_bishoplike_moves(u64 pinned_pieces) const;
  // Check if there are legal king moves given that the king is not in check.
  bool has_unchecked_king_moves() const;
  // Check if there are legal knight moves given that the king is not in check.
  bool has_unchecked_knight_moves(u64 pinned_pieces) const;
  // Check if there are legal pawn moves given that the king is not in check.
  bool has_unchecked_pawn_moves(u64 pinned_pieces) const;
  // Check if there are legal rook moves (and queen moves with rook movement) given that the king is not in check.
  bool has_unchecked_rooklike_moves(u64 pinned_pieces) const;

  // Check if there are legal king moves that escape the single-check.
  bool has_king_single_check_evasions(u64 attacker) const;
  // Check if there are legal king moves that escape the double-check.
  bool has_king_double_check_evasions() const;
};