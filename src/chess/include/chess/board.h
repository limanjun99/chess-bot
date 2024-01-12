#pragma once

#include <string_view>
#include <vector>

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
  std::vector<Move> generate_quiescence_moves() const;

  // Generate a list of all legal captures, checks and promotions.
  std::vector<Move> generate_quiescence_moves_and_checks() const;

  // Generate a list of all legal moves.
  std::vector<Move> generate_moves() const;

  // Generate a MoveSet representation of all legal moves. This is faster than `generate_moves` as it only computes move
  // legality on the fly when iterating through the MoveSet.
  MoveSet generate_moveset() const;

  // Check if the given move is a capture.
  bool is_a_capture(const Move& move) const;

  // Check if the given move is a check. Note that this function is not optimized.
  bool is_a_check(const Move& move) const;

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
  u64 en_passant_bit;  // The square that is currently capturable by en passant (if any).
  u64 cur_occupied;
  u64 opp_occupied;
  bool is_white_turn;

  // Either add the pawn move, or add all 4 possible promotions if it is a promotion.
  void add_pawn_moves(std::vector<Move>& moves, u64 from, u64 to) const;
  // Returns a bitboard of opponent pieces that attack the king.
  u64 get_king_attackers() const;
  // Returns a bitboard of my pieces that are pinned (i.e. removing them will open up the king to an attacker).
  u64 get_pinned_pieces() const;

  enum class MoveType { All, CapturesAndPromotionsOnly, CapturesChecksAndPromotionsOnly };
  // Generate legal bishop moves (and queen moves with bishop movement) given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_bishoplike_moves(std::vector<Move>& moves, u64 pinned_pieces) const;
  // Generate legal king moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_king_moves(std::vector<Move>& moves) const;
  // Generate legal knight moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_knight_moves(std::vector<Move>& moves, u64 pinned_pieces) const;
  // Generate legal pawn moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_pawn_moves(std::vector<Move>& moves, u64 pinned_pieces) const;
  // Generate legal rook moves (and queen moves with rook movement) given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_rooklike_moves(std::vector<Move>& moves, u64 pinned_pieces) const;

  // Generate legal king moves that escape the single-check.
  void generate_king_single_check_evasions(std::vector<Move>& moves, u64 attacker) const;
  // Generate legal king moves that escape the double-check.
  template <MoveType MT>
  void generate_king_double_check_evasions(std::vector<Move>& moves) const;

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