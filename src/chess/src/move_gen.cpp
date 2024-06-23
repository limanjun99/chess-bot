#include "move_gen.h"

#include <array>

#include "bitboard.h"
#include "piece.h"

using namespace chess;

template <Color PlayerColor>
class MoveGen {
public:
  MoveGen(const Board& board);

  // Generate a list of all legal moves.
  MoveContainer generate_moves() const;

  // Generate a list of all legal captures and promotions.
  MoveContainer generate_quiescence_moves() const;

  // Generate a list of all legal captures, checks and promotions.
  MoveContainer generate_quiescence_moves_and_checks() const;

  // Returns true if the current player still has any move to make.
  bool has_moves() const;

  // Check if the given square is under attack by the opponent.
  bool is_under_attack(Bitboard square) const;

private:
  const Board& board;
  const Player& cur_player;
  const Player& opp_player;
  const Bitboard cur_occupied;
  const Bitboard opp_occupied;
  const Bitboard total_occupied;
  const std::array<Bitboard, 3> opp_piece_at;
  const Bitboard king_bishop_rays;
  const Bitboard king_rook_rays;
  const Bitboard pinners;        // Opp pieces that pin my pieces to my king.
  const Bitboard pinned_pieces;  // My pieces that are pinned to my king.

  // Returns a bitboard of opp pieces that are pinning any of my pieces to my king. To be called during initialization.
  Bitboard compute_pinners() const;
  // Returns a bitboard of my pieces that are pinned to my king. To be called during initialization.
  Bitboard compute_pinned_pieces() const;

  // Either add the pawn push, or add all 4 possible promotions.
  void add_pawn_push(MoveContainer& moves, Bitboard from, Bitboard to) const;
  // Either add the pawn capture, or add all 4 possible promotion-captures. Note that this does not handle en-passant.
  void add_pawn_capture(MoveContainer& moves, Bitboard from, Bitboard to) const;
  // Returns a bitboard of opponent pieces that attack the king.
  Bitboard get_king_attackers() const;
  // Gets the opponent piece at the given square.
  PieceVariant get_opp_piece_at(Bitboard bit) const;

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
  void generate_king_single_check_evasions(MoveContainer& moves, Bitboard attacker) const;
  // Generate legal king moves that escape the double-check.
  template <MoveType MT>
  void generate_king_double_check_evasions(MoveContainer& moves) const;

  // TODO: The below methods `has_..._moves` are created solely for quickly checking if there are any more moves in a
  // position (without generating all moves). This is extremely ugly, and there is significant overlap with
  // `generate_..._moves`. Find a better way to do it.

  // Check if there are legal bishop moves (and queen moves with bishop movement) given that the king is not in check.
  bool has_unchecked_bishoplike_moves() const;
  // Check if there are legal king moves given that the king is not in check.
  bool has_unchecked_king_moves() const;
  // Check if there are legal knight moves given that the king is not in check.
  bool has_unchecked_knight_moves() const;
  // Check if there are legal pawn moves given that the king is not in check.
  bool has_unchecked_pawn_moves() const;
  // Check if there are legal rook moves (and queen moves with rook movement) given that the king is not in check.
  bool has_unchecked_rooklike_moves() const;

  // Check if there are legal king moves that escape the single-check.
  bool has_king_single_check_evasions(Bitboard attacker) const;
  // Check if there are legal king moves that escape the double-check.
  bool has_king_double_check_evasions() const;
};

namespace {
std::array<Bitboard, 3> compute_piece_at(const Player& player) {
  std::array<Bitboard, 3> piece_at{};
  // All the piece bitboards are compressed into 3 bitboards, where piece_at[i] represents whether the ith bit is set
  // for the piece at each square.
  const Bitboard none_bitboard = bitboard::ALL & ~(player.occupied());
  piece_at[0] = player[static_cast<PieceVariant>(1)] | player[static_cast<PieceVariant>(3)] |
                player[static_cast<PieceVariant>(5)];
  piece_at[1] = player[static_cast<PieceVariant>(2)] | player[static_cast<PieceVariant>(3)] | none_bitboard;
  piece_at[2] = player[static_cast<PieceVariant>(4)] | player[static_cast<PieceVariant>(5)] | none_bitboard;
  return piece_at;
}
}  // namespace

template <>
MoveGen<Color::White>::MoveGen(const Board& board)
    : board{board},
      cur_player{board.get_white()},
      opp_player{board.get_black()},
      cur_occupied{cur_player.occupied()},
      opp_occupied{opp_player.occupied()},
      total_occupied{cur_occupied | opp_occupied},
      opp_piece_at{compute_piece_at(opp_player)},
      king_bishop_rays{Bishop::attacks(cur_player[PieceVariant::King], total_occupied)},
      king_rook_rays{Rook::attacks(cur_player[PieceVariant::King], total_occupied)},
      pinners{compute_pinners()},
      pinned_pieces{compute_pinned_pieces()} {}

template <>
MoveGen<Color::Black>::MoveGen(const Board& board)
    : board{board},
      cur_player{board.get_black()},
      opp_player{board.get_white()},
      cur_occupied{cur_player.occupied()},
      opp_occupied{opp_player.occupied()},
      total_occupied{cur_occupied | opp_occupied},
      opp_piece_at{compute_piece_at(opp_player)},
      king_bishop_rays{Bishop::attacks(cur_player[PieceVariant::King], total_occupied)},
      king_rook_rays{Rook::attacks(cur_player[PieceVariant::King], total_occupied)},
      pinners{compute_pinners()},
      pinned_pieces{compute_pinned_pieces()} {}

template <Color PlayerColor>
MoveContainer MoveGen<PlayerColor>::generate_moves() const {
  MoveContainer moves;
  Bitboard king_attackers = get_king_attackers();
  if (!king_attackers) {
    // King is not in check.
    // Note that queen moves are generated in both generate_unchecked_bishoplike_moves() and
    // generate_unchecked_rooklike_moves().
    generate_unchecked_bishoplike_moves<MoveType::All>(moves);
    generate_unchecked_king_moves<MoveType::All>(moves);
    generate_unchecked_knight_moves<MoveType::All>(moves);
    generate_unchecked_pawn_moves<MoveType::All>(moves);
    generate_unchecked_rooklike_moves<MoveType::All>(moves);
  } else if (king_attackers.count() == 1) {
    // King is in single-check.
    generate_king_single_check_evasions(moves, king_attackers);
  } else {
    // King is in double-check.
    generate_king_double_check_evasions<MoveType::All>(moves);
  }
  return moves;
}

template <Color PlayerColor>
MoveContainer MoveGen<PlayerColor>::generate_quiescence_moves() const {
  MoveContainer moves;
  generate_unchecked_bishoplike_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_king_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_pawn_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_rooklike_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  return moves;
}

template <Color PlayerColor>
MoveContainer MoveGen<PlayerColor>::generate_quiescence_moves_and_checks() const {
  MoveContainer moves;
  generate_unchecked_bishoplike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_king_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_pawn_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_rooklike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);

  // The move generation above can only generate checks where the piece moved is the one giving check.
  // Another type of check is where the piece moves and another piece is the one giving check, which we generate below.
  const Bitboard opp_king = opp_player[PieceVariant::King];
  const Bitboard opp_king_bishop_rays = Bishop::attacks(opp_king, total_occupied);
  const Bitboard opp_king_rook_rays = Rook::attacks(opp_king, total_occupied);
  const Bitboard cur_king_attackers = get_king_attackers();

  // The piece at `from`, if moved to anywhere in `to_mask`, will cause a check by another piece.
  // Note that captures and promotions are ignored, as they are generated above already.
  auto generate_indirect_checks = [&](const Bitboard from, Bitboard to_mask) {
    const PieceVariant piece = cur_player.piece_at(from);
    if (pinned_pieces & from) {
      // The piece is pinned to its own king, can only move along the pin ray.
      const Bitboard new_king_attackers = Queen::attacks(cur_player[PieceVariant::King], total_occupied ^ from);
      const Bitboard pinner = new_king_attackers ^ cur_king_attackers;
      to_mask &= cur_player[PieceVariant::King].until(pinner);
    }
    to_mask &= ~total_occupied;
    switch (piece) {
      case PieceVariant::Bishop:
        to_mask &= ~opp_king_bishop_rays & Bishop::attacks(from, total_occupied);
        break;
      case PieceVariant::King:
        to_mask &= King::attacks(from);
        break;
      case PieceVariant::Knight:
        to_mask &= ~Knight::attacks(opp_player[PieceVariant::King]) & Knight::attacks(from);
        break;
      case PieceVariant::Pawn:
        to_mask &= ~Pawn::attacks<color::flip(PlayerColor)>(opp_player[PieceVariant::King]) & ~bitboard::RANK_1 &
                   ~bitboard::RANK_8 & Pawn::pushes<PlayerColor>(from, total_occupied);
        break;
      case PieceVariant::Queen:
        to_mask &= ~opp_king_bishop_rays & ~opp_king_rook_rays & Queen::attacks(from, total_occupied);
        break;
      case PieceVariant::Rook:
        to_mask &= ~opp_king_rook_rays & Rook::attacks(from, total_occupied);
        break;
      default:
        throw "Unreachable";
    }
    if (piece == PieceVariant::King) {
      // Check if the king is moving into check.
      for (const Bitboard to : to_mask.iterate()) {
        if (Bishop::attacks(to, total_occupied) & (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen]))
          continue;
        if (King::attacks(to) & opp_player[PieceVariant::King]) continue;
        if (Knight::attacks(to) & opp_player[PieceVariant::Knight]) continue;
        if (Pawn::attacks<PlayerColor>(to) & opp_player[PieceVariant::Pawn]) continue;
        if (Rook::attacks(to, total_occupied) & (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]))
          continue;
        moves.emplace_back(piece, from, to, get_opp_piece_at(to));
      }
    } else {
      for (const Bitboard to : to_mask.iterate()) {
        moves.emplace_back(piece, from, to, get_opp_piece_at(to));
      }
    }
  };

  const Bitboard on_opp_king_rook_rays = opp_king_rook_rays & cur_occupied;
  Bitboard rooks_attacking_opp_king = Rook::attacks(opp_king, total_occupied ^ on_opp_king_rook_rays) &
                                      (cur_player[PieceVariant::Rook] | cur_player[PieceVariant::Queen]);
  for (const Bitboard rook : rooks_attacking_opp_king.iterate()) {
    const Bitboard between_rook_and_opp_king = opp_king.until(rook);
    const Bitboard from = between_rook_and_opp_king & cur_occupied;
    const Bitboard to_mask = ~between_rook_and_opp_king;
    generate_indirect_checks(from, to_mask);
  }
  const Bitboard on_opp_king_bishop_rays = opp_king_bishop_rays & cur_occupied;
  Bitboard bishops_attacking_opp_king = Bishop::attacks(opp_king, total_occupied ^ on_opp_king_bishop_rays) &
                                        (cur_player[PieceVariant::Bishop] | cur_player[PieceVariant::Queen]);
  for (const Bitboard bishop : bishops_attacking_opp_king.iterate()) {
    const Bitboard between_bishop_and_opp_king = opp_king.until(bishop);
    const Bitboard from = between_bishop_and_opp_king & cur_occupied;
    const Bitboard to_mask = ~between_bishop_and_opp_king;
    generate_indirect_checks(from, to_mask);
  }

  return moves;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_moves() const {
  Bitboard king_attackers = get_king_attackers();
  if (!king_attackers) {
    // King is not in check.
    // Note that queen moves are checked in both has_unchecked_bishoplike_moves()
    // and has_unchecked_rooklike_moves().
    return has_unchecked_bishoplike_moves() || has_unchecked_king_moves() || has_unchecked_knight_moves() ||
           has_unchecked_pawn_moves() || has_unchecked_rooklike_moves();
  } else if (king_attackers.count() == 1) {
    // King is in single-check.
    return has_king_single_check_evasions(king_attackers);
  } else {
    // King is in double-check.
    return has_king_double_check_evasions();
  }
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::is_under_attack(Bitboard square) const {
  if (Bishop::attacks(square, total_occupied) & (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen])) {
    return true;
  }
  if (King::attacks(square) & opp_player[PieceVariant::King]) return true;
  if (Knight::attacks(square) & opp_player[PieceVariant::Knight]) return true;
  if (Pawn::attacks<PlayerColor>(square) & opp_player[PieceVariant::Pawn]) return true;
  if (Rook::attacks(square, total_occupied) & (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen])) {
    return true;
  }
  return false;
}

template <Color PlayerColor>
Bitboard MoveGen<PlayerColor>::compute_pinners() const {
  // NOTE: This function is called during initialization, not all members might be
  // initialized yet!
  const Bitboard current_slider_attackers =
      (king_bishop_rays & (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen])) |
      (king_rook_rays & (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]));
  const Bitboard potential_pinned_pieces = (king_bishop_rays | king_rook_rays) & cur_occupied;
  const Bitboard new_slider_attackers =
      (Bishop::attacks(cur_player[PieceVariant::King], total_occupied ^ potential_pinned_pieces) &
       (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen])) |
      (Rook::attacks(cur_player[PieceVariant::King], total_occupied ^ potential_pinned_pieces) &
       (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]));
  return new_slider_attackers ^ current_slider_attackers;
}

template <Color PlayerColor>
Bitboard MoveGen<PlayerColor>::compute_pinned_pieces() const {
  // NOTE: This function is called during initialization, not all members might be
  // initialized yet!
  Bitboard pinned_pieces{bitboard::EMPTY};
  for (const Bitboard pinner : pinners.iterate()) {
    const Bitboard pinned_piece = cur_player[PieceVariant::King].until(pinner) & cur_occupied;
    pinned_pieces ^= pinned_piece;
  }
  return pinned_pieces;
}

template <Color PlayerColor>
void MoveGen<PlayerColor>::add_pawn_push(MoveContainer& moves, Bitboard from, Bitboard to) const {
  if (to & (bitboard::RANK_1 | bitboard::RANK_8)) {
    moves.emplace_back(from, to, PieceVariant::Bishop, PieceVariant::None);
    moves.emplace_back(from, to, PieceVariant::Knight, PieceVariant::None);
    moves.emplace_back(from, to, PieceVariant::Queen, PieceVariant::None);
    moves.emplace_back(from, to, PieceVariant::Rook, PieceVariant::None);
  } else {
    moves.emplace_back(PieceVariant::Pawn, from, to, PieceVariant::None);
  }
}

template <Color PlayerColor>
void MoveGen<PlayerColor>::add_pawn_capture(MoveContainer& moves, Bitboard from, Bitboard to) const {
  PieceVariant captured = get_opp_piece_at(to);
  if (to & (bitboard::RANK_1 | bitboard::RANK_8)) {
    moves.emplace_back(from, to, PieceVariant::Bishop, captured);
    moves.emplace_back(from, to, PieceVariant::Knight, captured);
    moves.emplace_back(from, to, PieceVariant::Queen, captured);
    moves.emplace_back(from, to, PieceVariant::Rook, captured);
  } else {
    moves.emplace_back(PieceVariant::Pawn, from, to, captured);
  }
}

template <Color PlayerColor>
Bitboard MoveGen<PlayerColor>::get_king_attackers() const {
  Bitboard king = cur_player[PieceVariant::King];
  Bitboard attackers = (king_bishop_rays & (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen])) |
                       (Knight::attacks(king) & opp_player[PieceVariant::Knight]) |
                       (Pawn::attacks<PlayerColor>(king) & opp_player[PieceVariant::Pawn]) |
                       (king_rook_rays & (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]));
  return attackers;
}

template <Color PlayerColor>
PieceVariant MoveGen<PlayerColor>::get_opp_piece_at(Bitboard bit) const {
  int index{bit.to_index()};
  int piece = (static_cast<uint64_t>(opp_piece_at[0] >> index) & 1) +
              ((static_cast<uint64_t>(opp_piece_at[1] >> index) & 1) << 1) +
              ((static_cast<uint64_t>(opp_piece_at[2] >> index) & 1) << 2);
  return static_cast<PieceVariant>(piece);
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_unchecked_bishoplike_moves(MoveContainer& moves) const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  Bitboard to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | Bishop::attacks(opp_player[PieceVariant::King], total_occupied);
  }

  for (PieceVariant from_piece : {PieceVariant::Bishop, PieceVariant::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == PieceVariant::Queen) to_mask |= Queen::attacks(opp_player[PieceVariant::King], total_occupied);
    }
    const Bitboard unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    for (const Bitboard from : unpinned_pieces.iterate()) {
      const Bitboard to_bitboard = Bishop::attacks(from, total_occupied) & ~cur_occupied & to_mask;
      for (const Bitboard to : to_bitboard.iterate()) {
        moves.emplace_back(from_piece, from, to, get_opp_piece_at(to));
      }
    }

    const Bitboard pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player[from_piece];
    for (const Bitboard from : pinned_from_pieces.iterate()) {
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      const Bitboard to_bitboard = (cur_player[PieceVariant::King].until(pinned_by) | pinned_by) & ~from & to_mask;
      for (const Bitboard to : to_bitboard.iterate()) {
        moves.emplace_back(from_piece, from, to, get_opp_piece_at(to));
      }
    }
  }
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_unchecked_king_moves(MoveContainer& moves) const {
  // The king is not in check, hence it either moves to an unattacked square, or
  // we may castle.
  generate_king_double_check_evasions<MT>(moves);

  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) return;

  const Bitboard king = cur_player[PieceVariant::King];
  Bitboard rook_to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    rook_to_mask = Rook::attacks(opp_player[PieceVariant::King], total_occupied);
  }
  if (cur_player.can_castle_kingside() && !(total_occupied & (king << 1 | king << 2)) && !is_under_attack(king << 1) &&
      !is_under_attack(king << 2) && (rook_to_mask & king << 1)) {
    moves.emplace_back(PieceVariant::King, king, king << 2);
  }
  if (cur_player.can_castle_queenside() && !(total_occupied & (king >> 1 | king >> 2 | king >> 3)) &&
      !is_under_attack(king >> 1) && !is_under_attack(king >> 2) && (rook_to_mask & king >> 1)) {
    moves.emplace_back(PieceVariant::King, king, king >> 2);
  }
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_unchecked_knight_moves(MoveContainer& moves) const {
  // Pinned knights cannot move, while un-pinned knights can move anywhere.
  Bitboard knights = cur_player[PieceVariant::Knight] & ~pinned_pieces;
  Bitboard to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | Knight::attacks(opp_player[PieceVariant::King]);
  }

  for (const Bitboard from : knights.iterate()) {
    const Bitboard to_bitboard = Knight::attacks(from) & ~cur_occupied & to_mask;
    for (const Bitboard to : to_bitboard.iterate()) {
      moves.emplace_back(PieceVariant::Knight, from, to, get_opp_piece_at(to));
    }
  }
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_unchecked_pawn_moves(MoveContainer& moves) const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of
  // the pinner.
  Bitboard to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) {
    to_mask = opp_occupied | Pawn::get_promotion_squares<PlayerColor>();
  }
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = Pawn::attacks<color::flip(PlayerColor)>(opp_player[PieceVariant::King]) | opp_occupied |
              Pawn::get_promotion_squares<PlayerColor>();
  }

  const Bitboard unpinned_pawns = cur_player[PieceVariant::Pawn] & ~pinned_pieces;
  for (const Bitboard from : unpinned_pawns.iterate()) {
    const Bitboard capture_bitboard = Pawn::attacks<PlayerColor>(from) & opp_occupied & to_mask;
    for (const Bitboard to : capture_bitboard.iterate()) add_pawn_capture(moves, from, to);
    const Bitboard push_bitboard = Pawn::pushes<PlayerColor>(from, total_occupied) & to_mask;
    for (const Bitboard to : push_bitboard.iterate()) add_pawn_push(moves, from, to);
  }

  const Bitboard pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player[PieceVariant::Pawn];
  for (const Bitboard from : pinned_pawns.iterate()) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      if (!(pinned_by & Pawn::attacks<PlayerColor>(from))) continue;
      add_pawn_capture(moves, from, pinned_by);
    } else {
      // Pinned along a rook ray.
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      Bitboard to_bitboard = Pawn::pushes<PlayerColor>(from, total_occupied) & to_mask;
      to_bitboard &= cur_player[PieceVariant::King].until(pinned_by);
      for (const Bitboard to : to_bitboard.iterate()) add_pawn_push(moves, from, to);
    }
  }

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would
  // be wrongly found to be legal.
  if (board.get_en_passant()) {
    const Bitboard from_bitboard =
        Pawn::attacks<color::flip(PlayerColor)>(board.get_en_passant()) & cur_player[PieceVariant::Pawn];
    for (const Bitboard from : from_bitboard.iterate()) {
      const Bitboard captured_pawn =
          (PlayerColor == Color::White) ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
      if (Bishop::attacks(cur_player[PieceVariant::King],
                          total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen]))
        continue;
      if (Rook::attacks(cur_player[PieceVariant::King],
                        total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]))
        continue;
      moves.emplace_back(PieceVariant::Pawn, from, board.get_en_passant(), PieceVariant::Pawn);
    }
  }
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_unchecked_rooklike_moves(MoveContainer& moves) const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  Bitboard to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | Rook::attacks(opp_player[PieceVariant::King], total_occupied);
  }

  for (PieceVariant from_piece : {PieceVariant::Rook, PieceVariant::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == PieceVariant::Queen) to_mask |= Queen::attacks(opp_player[PieceVariant::King], total_occupied);
    }
    const Bitboard unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    for (const Bitboard from : unpinned_pieces.iterate()) {
      const Bitboard to_bitboard = Rook::attacks(from, total_occupied) & ~cur_occupied & to_mask;
      for (const Bitboard to : to_bitboard.iterate()) {
        moves.emplace_back(from_piece, from, to, get_opp_piece_at(to));
      }
    }

    const Bitboard pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player[from_piece];
    for (const Bitboard from : pinned_from_pieces.iterate()) {
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      const Bitboard to_bitboard = (cur_player[PieceVariant::King].until(pinned_by) | pinned_by) & ~from & to_mask;
      for (const Bitboard to : to_bitboard.iterate()) {
        moves.emplace_back(from_piece, from, to, get_opp_piece_at(to));
      }
    }
  }
}

template <Color PlayerColor>
void MoveGen<PlayerColor>::generate_king_single_check_evasions(MoveContainer& moves, Bitboard attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  // 1. Capture the attacker with a piece that is not pinned.
  const Bitboard attacker_bishop_rays = Bishop::attacks(attacker, total_occupied);
  const Bitboard attacker_rook_rays = Rook::attacks(attacker, total_occupied);
  const PieceVariant attacker_piece = get_opp_piece_at(attacker);
  // Capture the attacker with a bishop that is not pinned.
  Bitboard bishop_capturers = attacker_bishop_rays & cur_player[PieceVariant::Bishop] & ~pinned_pieces;
  for (const Bitboard from : bishop_capturers.iterate()) {
    moves.emplace_back(PieceVariant::Bishop, from, attacker, attacker_piece);
  }
  // Capture the attacker with a knight that is not pinned.
  Bitboard knight_capturers = Knight::attacks(attacker) & cur_player[PieceVariant::Knight] & ~pinned_pieces;
  for (const Bitboard from : knight_capturers.iterate()) {
    moves.emplace_back(PieceVariant::Knight, from, attacker, attacker_piece);
  }
  // Capture the attacker with a pawn that is not pinned.
  Bitboard pawn_capturers =
      Pawn::attacks<color::flip(PlayerColor)>(attacker) & cur_player[PieceVariant::Pawn] & ~pinned_pieces;
  for (const Bitboard from : pawn_capturers.iterate()) {
    add_pawn_capture(moves, from, attacker);
  }
  // Special en-passant check for pawns.
  if (attacker == (PlayerColor == Color::White ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    Bitboard capturing_square = PlayerColor == Color::White ? attacker << 8 : attacker >> 8;
    Bitboard pawn_capturers =
        Pawn::attacks<color::flip(PlayerColor)>(capturing_square) & cur_player[PieceVariant::Pawn] & ~pinned_pieces;
    for (const Bitboard from : pawn_capturers.iterate()) {
      moves.emplace_back(PieceVariant::Pawn, from, capturing_square, attacker_piece);
    }
  }
  // Capture the attacker with a queen that is not pinned.
  Bitboard queen_capturers =
      (attacker_bishop_rays | attacker_rook_rays) & cur_player[PieceVariant::Queen] & ~pinned_pieces;
  for (const Bitboard from : queen_capturers.iterate()) {
    moves.emplace_back(PieceVariant::Queen, from, attacker, attacker_piece);
  }
  // Capture the attacker with a rook that is not pinned.
  Bitboard rook_capturers = attacker_rook_rays & cur_player[PieceVariant::Rook] & ~pinned_pieces;
  for (const Bitboard from : rook_capturers.iterate()) {
    moves.emplace_back(PieceVariant::Rook, from, attacker, attacker_piece);
  }

  // 2. King moves to a square that is not attacked. This is the same as evading
  // double check.
  generate_king_double_check_evasions<MoveType::All>(moves);

  // 3. Block the check if it is a sliding check.
  if (piece_variant::is_slider(attacker_piece)) {
    Bitboard blocking_squares = cur_player[PieceVariant::King].until(attacker);
    for (const Bitboard to : blocking_squares.iterate()) {
      const Bitboard blocker_bishop_rays = Bishop::attacks(to, total_occupied);
      const Bitboard blocker_rook_rays = Rook::attacks(to, total_occupied);
      // Block with a bishop that is not pinned.
      Bitboard bishop_blockers = blocker_bishop_rays & cur_player[PieceVariant::Bishop] & ~pinned_pieces;
      for (const Bitboard from : bishop_blockers.iterate()) {
        moves.emplace_back(PieceVariant::Bishop, from, to);
      }
      // Block with a knight that is not pinned.
      Bitboard knight_blockers = Knight::attacks(to) & cur_player[PieceVariant::Knight] & ~pinned_pieces;
      for (const Bitboard from : knight_blockers.iterate()) {
        moves.emplace_back(PieceVariant::Knight, from, to);
      }
      // Block with a pawn that is not pinned.
      Bitboard pawn_blockers = (PlayerColor == Color::White ? to >> 8 : to << 8);
      if constexpr (PlayerColor == Color::White) {
        pawn_blockers |= ((to >> 8) & ~total_occupied) >> 8 & bitboard::RANK_2;
      } else {
        pawn_blockers |= ((to << 8) & ~total_occupied) << 8 & bitboard::RANK_7;
      }
      pawn_blockers = pawn_blockers & cur_player[PieceVariant::Pawn] & ~pinned_pieces;
      for (const Bitboard from : pawn_blockers.iterate()) {
        moves.emplace_back(PieceVariant::Pawn, from, to);
      }
      // Block with a queen that is not pinned.
      Bitboard queen_blockers =
          (blocker_bishop_rays | blocker_rook_rays) & cur_player[PieceVariant::Queen] & ~pinned_pieces;
      for (const Bitboard from : queen_blockers.iterate()) {
        moves.emplace_back(PieceVariant::Queen, from, to);
      }
      // Block with a rook that is not pinned.
      Bitboard rook_blockers = blocker_rook_rays & cur_player[PieceVariant::Rook] & ~pinned_pieces;
      for (const Bitboard from : rook_blockers.iterate()) {
        moves.emplace_back(PieceVariant::Rook, from, to);
      }
    }
  }
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_king_double_check_evasions(MoveContainer& moves) const {
  // To evade a double check, the king must move to a square that is not attacked.
  const Bitboard king = cur_player[PieceVariant::King];
  Bitboard to_bitboard = King::attacks(king) & ~cur_occupied &
                         ~Pawn::attacks<color::flip(PlayerColor)>(opp_player[PieceVariant::Pawn]) &
                         ~King::attacks(opp_player[PieceVariant::King]);
  if constexpr (MT != MoveType::All) to_bitboard &= opp_occupied;
  const Bitboard total_occupied_without_king = total_occupied ^ king;
  for (const Bitboard to : to_bitboard.iterate()) {
    if (Knight::attacks(to) & opp_player[PieceVariant::Knight]) continue;
    if (Bishop::attacks(to, total_occupied_without_king) &
        (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen]))
      continue;
    if (Rook::attacks(to, total_occupied_without_king) &
        (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]))
      continue;
    moves.emplace_back(PieceVariant::King, king, to, get_opp_piece_at(to));
  }
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_unchecked_bishoplike_moves() const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  for (PieceVariant from_piece : {PieceVariant::Bishop, PieceVariant::Queen}) {
    const Bitboard unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    for (const Bitboard from : unpinned_pieces.iterate()) {
      const Bitboard to_bitboard = Bishop::attacks(from, total_occupied) & ~cur_occupied;
      if (to_bitboard) return true;
    }

    const Bitboard pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player[from_piece];
    for (const Bitboard from : pinned_from_pieces.iterate()) {
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      const Bitboard to_bitboard = (cur_player[PieceVariant::King].until(pinned_by) | pinned_by) & ~from;
      if (to_bitboard) return true;
    }
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_unchecked_king_moves() const {
  // The king is not in check, hence it either moves to an unattacked square, or
  // we may castle.
  if (has_king_double_check_evasions()) return true;

  const Bitboard king = cur_player[PieceVariant::King];
  if (cur_player.can_castle_kingside() && !(total_occupied & (king << 1 | king << 2)) && !is_under_attack(king << 1) &&
      !is_under_attack(king << 2)) {
    return true;
  }
  if (cur_player.can_castle_queenside() && !(total_occupied & (king >> 1 | king >> 2 | king >> 3)) &&
      !is_under_attack(king >> 1) && !is_under_attack(king >> 2)) {
    return true;
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_unchecked_knight_moves() const {
  // Pinned knights cannot move, while un-pinned knights can move anywhere.
  Bitboard knights = cur_player[PieceVariant::Knight] & ~pinned_pieces;
  for (const Bitboard from : knights.iterate()) {
    Bitboard to_bitboard = Knight::attacks(from) & ~cur_occupied;
    if (to_bitboard) return true;
  }
  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_unchecked_pawn_moves() const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of
  // the pinner.
  const Bitboard unpinned_pawns = cur_player[PieceVariant::Pawn] & ~pinned_pieces;
  for (const Bitboard from : unpinned_pawns.iterate()) {
    const Bitboard capture_bitboard = Pawn::attacks<PlayerColor>(from) & opp_occupied;
    const Bitboard push_bitboard = Pawn::pushes<PlayerColor>(from, total_occupied);
    if (push_bitboard | capture_bitboard) return true;
  }

  const Bitboard pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player[PieceVariant::Pawn];
  for (const Bitboard from : pinned_pawns.iterate()) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      if (pinned_by & Pawn::attacks<PlayerColor>(from)) return true;
    } else {
      // Pinned along a rook ray.
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      Bitboard to_bitboard = Pawn::pushes<PlayerColor>(from, total_occupied);
      to_bitboard &= cur_player[PieceVariant::King].until(pinned_by);
      if (to_bitboard) return true;
    }
  }

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would
  // be wrongly found to be legal.
  if (board.get_en_passant()) {
    const Bitboard from_bitboard =
        Pawn::attacks<color::flip(PlayerColor)>(board.get_en_passant()) & cur_player[PieceVariant::Pawn];
    for (const Bitboard from : from_bitboard.iterate()) {
      const Bitboard captured_pawn =
          PlayerColor == Color::White ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
      if (Bishop::attacks(cur_player[PieceVariant::King],
                          total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen]))
        continue;
      if (Rook::attacks(cur_player[PieceVariant::King],
                        total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]))
        continue;
      return true;
    }
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_unchecked_rooklike_moves() const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  for (PieceVariant from_piece : {PieceVariant::Rook, PieceVariant::Queen}) {
    const Bitboard unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    for (const Bitboard from : unpinned_pieces.iterate()) {
      const Bitboard to_bitboard = Rook::attacks(from, total_occupied) & ~cur_occupied;
      if (to_bitboard) return true;
    }

    const Bitboard pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player[from_piece];
    for (const Bitboard from : pinned_from_pieces.iterate()) {
      const Bitboard pinned_by = cur_player[PieceVariant::King].beyond(from) & pinners;
      const Bitboard to_bitboard = (cur_player[PieceVariant::King].until(pinned_by) | pinned_by) & ~from;
      if (to_bitboard) return true;
    }
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_king_single_check_evasions(Bitboard attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  // 1. Capture the attacker with a piece that is not pinned.
  const Bitboard attacker_bishop_rays = Bishop::attacks(attacker, total_occupied);
  const Bitboard attacker_rook_rays = Rook::attacks(attacker, total_occupied);
  // Capture the attacker with a bishop that is not pinned.
  Bitboard bishop_capturers = attacker_bishop_rays & cur_player[PieceVariant::Bishop] & ~pinned_pieces;
  if (bishop_capturers) return true;
  // Capture the attacker with a knight that is not pinned.
  Bitboard knight_capturers = Knight::attacks(attacker) & cur_player[PieceVariant::Knight] & ~pinned_pieces;
  if (knight_capturers) return true;
  // Capture the attacker with a pawn that is not pinned.
  Bitboard pawn_capturers =
      Pawn::attacks<color::flip(PlayerColor)>(attacker) & cur_player[PieceVariant::Pawn] & ~pinned_pieces;
  if (pawn_capturers) return true;
  // Special en-passant check for pawns.
  if (attacker == (PlayerColor == Color::White ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    Bitboard capturing_square = PlayerColor == Color::White ? attacker << 8 : attacker >> 8;
    Bitboard pawn_capturers =
        Pawn::attacks<color::flip(PlayerColor)>(capturing_square) & cur_player[PieceVariant::Pawn] & ~pinned_pieces;
    if (pawn_capturers) return true;
  }
  // Capture the attacker with a queen that is not pinned.
  Bitboard queen_capturers =
      (attacker_bishop_rays | attacker_rook_rays) & cur_player[PieceVariant::Queen] & ~pinned_pieces;
  if (queen_capturers) return true;
  // Capture the attacker with a rook that is not pinned.
  Bitboard rook_capturers = attacker_rook_rays & cur_player[PieceVariant::Rook] & ~pinned_pieces;
  if (rook_capturers) return true;

  // 2. King moves to a square that is not attacked. This is the same as evading
  // double check.
  if (has_king_double_check_evasions()) return true;

  // 3. Block the check if it is a sliding check.
  const PieceVariant attacker_piece = get_opp_piece_at(attacker);
  if (piece_variant::is_slider(attacker_piece)) {
    Bitboard blocking_squares = cur_player[PieceVariant::King].until(attacker);
    for (const Bitboard to : blocking_squares.iterate()) {
      const Bitboard blocker_bishop_rays = Bishop::attacks(to, total_occupied);
      const Bitboard blocker_rook_rays = Rook::attacks(to, total_occupied);
      // Block with a bishop that is not pinned.
      Bitboard bishop_blockers = blocker_bishop_rays & cur_player[PieceVariant::Bishop] & ~pinned_pieces;
      if (bishop_blockers) return true;
      // Block with a knight that is not pinned.
      Bitboard knight_blockers = Knight::attacks(to) & cur_player[PieceVariant::Knight] & ~pinned_pieces;
      if (knight_blockers) return true;
      // Block with a pawn that is not pinned.
      Bitboard pawn_blockers = (PlayerColor == Color::White ? to >> 8 : to << 8);
      if constexpr (PlayerColor == Color::White) {
        pawn_blockers |= ((to >> 8) & ~total_occupied) >> 8 & bitboard::RANK_2;
      } else {
        pawn_blockers |= ((to << 8) & ~total_occupied) << 8 & bitboard::RANK_7;
      }
      pawn_blockers = pawn_blockers & cur_player[PieceVariant::Pawn] & ~pinned_pieces;
      if (pawn_blockers) return true;
      // Block with a queen that is not pinned.
      Bitboard queen_blockers =
          (blocker_bishop_rays | blocker_rook_rays) & cur_player[PieceVariant::Queen] & ~pinned_pieces;
      if (queen_blockers) return true;
      // Block with a rook that is not pinned.
      Bitboard rook_blockers = blocker_rook_rays & cur_player[PieceVariant::Rook] & ~pinned_pieces;
      if (rook_blockers) return true;
    }
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_king_double_check_evasions() const {
  // To evade a double check, the king must move to a square that is not attacked.
  const Bitboard king = cur_player[PieceVariant::King];
  const Bitboard to_bitboard = King::attacks(king) & ~cur_occupied &
                               ~Pawn::attacks<color::flip(PlayerColor)>(opp_player[PieceVariant::Pawn]) &
                               ~King::attacks(opp_player[PieceVariant::King]);
  const Bitboard total_occupied_without_king = total_occupied ^ king;
  for (const Bitboard to : to_bitboard.iterate()) {
    if (Knight::attacks(to) & opp_player[PieceVariant::Knight]) continue;
    if (Bishop::attacks(to, total_occupied_without_king) &
        (opp_player[PieceVariant::Bishop] | opp_player[PieceVariant::Queen]))
      continue;
    if (Rook::attacks(to, total_occupied_without_king) &
        (opp_player[PieceVariant::Rook] | opp_player[PieceVariant::Queen]))
      continue;
    return true;
  }
  return false;
}

MoveContainer move_gen::generate_moves(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<Color::White>{board}.generate_moves();
  } else {
    return MoveGen<Color::Black>{board}.generate_moves();
  }
}

MoveContainer move_gen::generate_quiescence_moves(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<Color::White>{board}.generate_quiescence_moves();
  } else {
    return MoveGen<Color::Black>{board}.generate_quiescence_moves();
  }
}

MoveContainer move_gen::generate_quiescence_moves_and_checks(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<Color::White>{board}.generate_quiescence_moves_and_checks();
  } else {
    return MoveGen<Color::Black>{board}.generate_quiescence_moves_and_checks();
  }
}

bool move_gen::has_moves(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<Color::White>{board}.has_moves();
  } else {
    return MoveGen<Color::Black>{board}.has_moves();
  }
}

bool move_gen::is_under_attack(const Board& board, Bitboard square) {
  if (board.is_white_to_move()) {
    return MoveGen<Color::White>{board}.is_under_attack(square);
  } else {
    return MoveGen<Color::Black>{board}.is_under_attack(square);
  }
}
