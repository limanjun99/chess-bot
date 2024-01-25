#pragma once

#include <array>

#include "board.h"

class MoveGen {
public:
  MoveGen(const Board& board);

  // Generate a list of all legal captures and promotions.
  MoveContainer generate_quiescence_moves() const;

  // Generate a list of all legal captures, checks and promotions.
  MoveContainer generate_quiescence_moves_and_checks() const;

  // Generate a list of all legal moves.
  MoveContainer generate_moves() const;

  // Returns true if the current player still has any move to make.
  bool has_moves() const;

  // Check if the given square is under attack by the opponent.
  bool is_under_attack(u64 square) const;

private:
  const Board& board;
  const Player& cur_player;
  const Player& opp_player;
  const u64 cur_occupied;
  const u64 opp_occupied;
  const u64 total_occupied;
  const std::array<u64, 3> opp_piece_at;
  const u64 king_bishop_rays;
  const u64 king_rook_rays;
  const u64 pinned_pieces;

  // Either add the pawn move, or add all 4 possible promotions if it is a promotion.
  void add_pawn_moves(MoveContainer& moves, u64 from, u64 to) const;
  // Returns a bitboard of opponent pieces that attack the king.
  u64 get_king_attackers() const;
  // Returns a bitboard of my pieces that are pinned (i.e. removing them will open up the king to an attacker).
  u64 get_pinned_pieces() const;
  // Gets the opponent piece at the given square.
  Piece get_opp_piece_at(u64 bit) const;

  enum class MoveType { All, CapturesAndPromotionsOnly, CapturesChecksAndPromotionsOnly };
  // Generate legal bishop moves (and queen moves with bishop movement) given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_bishoplike_moves(MoveContainer& moves) const;
  // Generate legal king moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_king_moves(MoveContainer& moves) const;
  // Generate legal knight moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_knight_moves(MoveContainer& moves) const;
  // Generate legal pawn moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_pawn_moves(MoveContainer& moves) const;
  // Generate legal rook moves (and queen moves with rook movement) given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_rooklike_moves(MoveContainer& moves) const;

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