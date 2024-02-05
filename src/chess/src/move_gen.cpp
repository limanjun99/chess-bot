#include "move_gen.h"

#include <array>

template <bool IsWhite>
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
  const u64 pinners;        // Opp pieces that pin my pieces to my king.
  const u64 pinned_pieces;  // My pieces that are pinned to my king.

  // Returns a bitboard of opp pieces that are pinning any of my pieces to my king. To be called during initialization.
  u64 compute_pinners() const;
  // Returns a bitboard of my pieces that are pinned to my king. To be called during initialization.
  u64 compute_pinned_pieces() const;

  // Either add the pawn push, or add all 4 possible promotions.
  void add_pawn_push(MoveContainer& moves, u64 from, u64 to) const;
  // Either add the pawn capture, or add all 4 possible promotion-captures. Note that this does not handle en-passant.
  void add_pawn_capture(MoveContainer& moves, u64 from, u64 to) const;
  // Returns a bitboard of opponent pieces that attack the king.
  u64 get_king_attackers() const;
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

namespace {
std::array<u64, 3> compute_piece_at(const Player& player) {
  std::array<u64, 3> piece_at{0, 0, 0};
  // All the piece bitboards are compressed into 3 bitboards, where piece_at[i] represents whether the ith bit is set
  // for the piece at each square.
  const u64 none_bitboard = bitboard::ALL & ~(player.occupied());
  piece_at[0] = player[static_cast<Piece>(1)] | player[static_cast<Piece>(3)] | player[static_cast<Piece>(5)];
  piece_at[1] = player[static_cast<Piece>(2)] | player[static_cast<Piece>(3)] | none_bitboard;
  piece_at[2] = player[static_cast<Piece>(4)] | player[static_cast<Piece>(5)] | none_bitboard;
  return piece_at;
}
}  // namespace

template <>
MoveGen<true>::MoveGen(const Board& board)
    : board{board},
      cur_player{board.get_white()},
      opp_player{board.get_black()},
      cur_occupied{cur_player.occupied()},
      opp_occupied{opp_player.occupied()},
      total_occupied{cur_occupied | opp_occupied},
      opp_piece_at{compute_piece_at(opp_player)},
      king_bishop_rays{bitboard::bishop_attacks(cur_player[Piece::King], total_occupied)},
      king_rook_rays{bitboard::rook_attacks(cur_player[Piece::King], total_occupied)},
      pinners{compute_pinners()},
      pinned_pieces{compute_pinned_pieces()} {}

template <>
MoveGen<false>::MoveGen(const Board& board)
    : board{board},
      cur_player{board.get_black()},
      opp_player{board.get_white()},
      cur_occupied{cur_player.occupied()},
      opp_occupied{opp_player.occupied()},
      total_occupied{cur_occupied | opp_occupied},
      opp_piece_at{compute_piece_at(opp_player)},
      king_bishop_rays{bitboard::bishop_attacks(cur_player[Piece::King], total_occupied)},
      king_rook_rays{bitboard::rook_attacks(cur_player[Piece::King], total_occupied)},
      pinners{compute_pinners()},
      pinned_pieces{compute_pinned_pieces()} {}

template <bool IsWhite>
MoveContainer MoveGen<IsWhite>::generate_moves() const {
  MoveContainer moves;
  u64 king_attackers = get_king_attackers();
  if (king_attackers == 0) {
    // King is not in check.
    // Note that queen moves are generated in both generate_unchecked_bishoplike_moves() and
    // generate_unchecked_rooklike_moves().
    generate_unchecked_bishoplike_moves<MoveType::All>(moves);
    generate_unchecked_king_moves<MoveType::All>(moves);
    generate_unchecked_knight_moves<MoveType::All>(moves);
    generate_unchecked_pawn_moves<MoveType::All>(moves);
    generate_unchecked_rooklike_moves<MoveType::All>(moves);
  } else if (bitboard::count(king_attackers) == 1) {
    // King is in single-check.
    generate_king_single_check_evasions(moves, king_attackers);
  } else {
    // King is in double-check.
    generate_king_double_check_evasions<MoveType::All>(moves);
  }
  return moves;
}

template <bool IsWhite>
MoveContainer MoveGen<IsWhite>::generate_quiescence_moves() const {
  MoveContainer moves;
  generate_unchecked_bishoplike_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_king_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_pawn_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_rooklike_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  return moves;
}

template <bool IsWhite>
MoveContainer MoveGen<IsWhite>::generate_quiescence_moves_and_checks() const {
  MoveContainer moves;
  generate_unchecked_bishoplike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_king_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_pawn_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_rooklike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);

  // The move generation above can only generate checks where the piece moved is the one giving check.
  // Another type of check is where the piece moves and another piece is the one giving check, which we generate below.
  const u64 opp_king = opp_player[Piece::King];
  const u64 opp_king_bishop_rays = bitboard::bishop_attacks(opp_king, total_occupied);
  const u64 opp_king_rook_rays = bitboard::rook_attacks(opp_king, total_occupied);
  const u64 cur_king_attackers = get_king_attackers();

  // The piece at `from`, if moved to anywhere in `to_mask`, will cause a check by another piece.
  // Note that captures and promotions are ignored, as they are generated above already.
  auto generate_indirect_checks = [&](const u64 from, u64 to_mask) {
    const Piece piece = cur_player.piece_at(from);
    if (pinned_pieces & from) {
      // The piece is pinned to its own king, can only move along the pin ray.
      const u64 new_king_attackers = bitboard::queen_attacks(cur_player[Piece::King], total_occupied ^ from);
      const u64 pinner = new_king_attackers ^ cur_king_attackers;
      to_mask &= bitboard::between(cur_player[Piece::King], pinner);
    }
    to_mask &= ~total_occupied;
    switch (piece) {
      case Piece::Bishop:
        to_mask &= ~opp_king_bishop_rays & bitboard::bishop_attacks(from, total_occupied);
        break;
      case Piece::King:
        to_mask &= bitboard::king_attacks(from);
        break;
      case Piece::Knight:
        to_mask &= ~bitboard::knight_attacks(opp_player[Piece::King]) & bitboard::knight_attacks(from);
        break;
      case Piece::Pawn:
        to_mask &= ~bitboard::pawn_attacks<!IsWhite>(opp_player[Piece::King]) & ~bitboard::RANK_1 & ~bitboard::RANK_8 &
                   bitboard::pawn_pushes<IsWhite>(from, total_occupied);
        break;
      case Piece::Queen:
        to_mask &= ~opp_king_bishop_rays & ~opp_king_rook_rays & bitboard::queen_attacks(from, total_occupied);
        break;
      case Piece::Rook:
        to_mask &= ~opp_king_rook_rays & bitboard::rook_attacks(from, total_occupied);
        break;
      default:
        throw "Unreachable";
    }
    if (piece == Piece::King) {
      // Check if the king is moving into check.
      BITBOARD_ITERATE(to_mask, to) {
        if (bitboard::bishop_attacks(to, total_occupied) & (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
          continue;
        if (bitboard::king_attacks(to) & opp_player[Piece::King]) continue;
        if (bitboard::knight_attacks(to) & opp_player[Piece::Knight]) continue;
        if (bitboard::pawn_attacks<IsWhite>(to) & opp_player[Piece::Pawn]) continue;
        if (bitboard::rook_attacks(to, total_occupied) & (opp_player[Piece::Rook] | opp_player[Piece::Queen])) continue;
        moves.emplace_back(piece, from, to, get_opp_piece_at(to));
      }
    } else {
      BITBOARD_ITERATE(to_mask, to) { moves.emplace_back(piece, from, to, get_opp_piece_at(to)); }
    }
  };

  const u64 on_opp_king_rook_rays = opp_king_rook_rays & cur_occupied;
  u64 rooks_attacking_opp_king = bitboard::rook_attacks(opp_king, total_occupied ^ on_opp_king_rook_rays) &
                                 (cur_player[Piece::Rook] | cur_player[Piece::Queen]);
  BITBOARD_ITERATE(rooks_attacking_opp_king, rook) {
    const u64 between_rook_and_opp_king = bitboard::between(opp_king, rook);
    const u64 from = between_rook_and_opp_king & cur_occupied;
    const u64 to_mask = ~between_rook_and_opp_king;
    generate_indirect_checks(from, to_mask);
  }
  const u64 on_opp_king_bishop_rays = opp_king_bishop_rays & cur_occupied;
  u64 bishops_attacking_opp_king = bitboard::bishop_attacks(opp_king, total_occupied ^ on_opp_king_bishop_rays) &
                                   (cur_player[Piece::Bishop] | cur_player[Piece::Queen]);
  BITBOARD_ITERATE(bishops_attacking_opp_king, bishop) {
    const u64 between_bishop_and_opp_king = bitboard::between(opp_king, bishop);
    const u64 from = between_bishop_and_opp_king & cur_occupied;
    const u64 to_mask = ~between_bishop_and_opp_king;
    generate_indirect_checks(from, to_mask);
  }

  return moves;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_moves() const {
  u64 king_attackers = get_king_attackers();
  if (king_attackers == 0) {
    // King is not in check.
    // Note that queen moves are checked in both has_unchecked_bishoplike_moves() and
    // has_unchecked_rooklike_moves().
    return has_unchecked_bishoplike_moves(pinned_pieces) || has_unchecked_king_moves() ||
           has_unchecked_knight_moves(pinned_pieces) || has_unchecked_pawn_moves(pinned_pieces) ||
           has_unchecked_rooklike_moves(pinned_pieces);
  } else if (bitboard::count(king_attackers) == 1) {
    // King is in single-check.
    return has_king_single_check_evasions(king_attackers);
  } else {
    // King is in double-check.
    return has_king_double_check_evasions();
  }
}

template <bool IsWhite>
bool MoveGen<IsWhite>::is_under_attack(u64 square) const {
  if (bitboard::bishop_attacks(square, total_occupied) & (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) {
    return true;
  }
  if (bitboard::king_attacks(square) & opp_player[Piece::King]) return true;
  if (bitboard::knight_attacks(square) & opp_player[Piece::Knight]) return true;
  if (bitboard::pawn_attacks<IsWhite>(square) & opp_player[Piece::Pawn]) return true;
  if (bitboard::rook_attacks(square, total_occupied) & (opp_player[Piece::Rook] | opp_player[Piece::Queen])) {
    return true;
  }
  return false;
}

template <bool IsWhite>
u64 MoveGen<IsWhite>::compute_pinners() const {
  // NOTE: This function is called during initialization, not all members might be initialized yet!
  const u64 current_slider_attackers = (king_bishop_rays & (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) |
                                       (king_rook_rays & (opp_player[Piece::Rook] | opp_player[Piece::Queen]));
  const u64 potential_pinned_pieces = (king_bishop_rays | king_rook_rays) & cur_occupied;
  const u64 new_slider_attackers =
      (bitboard::bishop_attacks(cur_player[Piece::King], total_occupied ^ potential_pinned_pieces) &
       (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) |
      (bitboard::rook_attacks(cur_player[Piece::King], total_occupied ^ potential_pinned_pieces) &
       (opp_player[Piece::Rook] | opp_player[Piece::Queen]));
  return new_slider_attackers ^ current_slider_attackers;
}

template <bool IsWhite>
u64 MoveGen<IsWhite>::compute_pinned_pieces() const {
  // NOTE: This function is called during initialization, not all members might be initialized yet!
  u64 pinned_pieces = 0;
  BITBOARD_ITERATE(pinners, pinner) {
    const u64 pinned_piece = bitboard::between(cur_player[Piece::King], pinner) & cur_occupied;
    pinned_pieces ^= pinned_piece;
  }
  return pinned_pieces;
}

template <bool IsWhite>
void MoveGen<IsWhite>::add_pawn_push(MoveContainer& moves, u64 from, u64 to) const {
  if (to & (bitboard::RANK_1 | bitboard::RANK_8)) {
    moves.emplace_back(from, to, Piece::Bishop, Piece::None);
    moves.emplace_back(from, to, Piece::Knight, Piece::None);
    moves.emplace_back(from, to, Piece::Queen, Piece::None);
    moves.emplace_back(from, to, Piece::Rook, Piece::None);
  } else {
    moves.emplace_back(Piece::Pawn, from, to, Piece::None);
  }
}

template <bool IsWhite>
void MoveGen<IsWhite>::add_pawn_capture(MoveContainer& moves, u64 from, u64 to) const {
  Piece captured = get_opp_piece_at(to);
  if (to & (bitboard::RANK_1 | bitboard::RANK_8)) {
    moves.emplace_back(from, to, Piece::Bishop, captured);
    moves.emplace_back(from, to, Piece::Knight, captured);
    moves.emplace_back(from, to, Piece::Queen, captured);
    moves.emplace_back(from, to, Piece::Rook, captured);
  } else {
    moves.emplace_back(Piece::Pawn, from, to, captured);
  }
}

template <bool IsWhite>
u64 MoveGen<IsWhite>::get_king_attackers() const {
  u64 king = cur_player[Piece::King];
  u64 attackers = (king_bishop_rays & (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) |
                  (bitboard::knight_attacks(king) & opp_player[Piece::Knight]) |
                  (bitboard::pawn_attacks<IsWhite>(king) & opp_player[Piece::Pawn]) |
                  (king_rook_rays & (opp_player[Piece::Rook] | opp_player[Piece::Queen]));
  return attackers;
}

template <bool IsWhite>
Piece MoveGen<IsWhite>::get_opp_piece_at(u64 bit) const {
  int index = bit::to_index(bit);
  int piece =
      (opp_piece_at[0] >> index & 1) + ((opp_piece_at[1] >> index & 1) << 1) + ((opp_piece_at[2] >> index & 1) << 2);
  return static_cast<Piece>(piece);
}

template <bool IsWhite>
template <typename MoveGen<IsWhite>::MoveType MT>
void MoveGen<IsWhite>::generate_unchecked_bishoplike_moves(MoveContainer& moves) const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::bishop_attacks(opp_player[Piece::King], total_occupied);
  }

  for (Piece from_piece : {Piece::Bishop, Piece::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == Piece::Queen) to_mask |= bitboard::queen_attacks(opp_player[Piece::King], total_occupied);
    }
    const u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      const u64 to_bitboard = bitboard::bishop_attacks(from, total_occupied) & ~cur_occupied & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }

    const u64 pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      const u64 to_bitboard = (bitboard::between(cur_player[Piece::King], pinned_by) | pinned_by) & ~from & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }
  }
}

template <bool IsWhite>
template <typename MoveGen<IsWhite>::MoveType MT>
void MoveGen<IsWhite>::generate_unchecked_king_moves(MoveContainer& moves) const {
  // The king is not in check, hence it either moves to an unattacked square, or we may castle.
  generate_king_double_check_evasions<MT>(moves);

  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) return;

  const u64 king = cur_player[Piece::King];
  u64 rook_to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    rook_to_mask = bitboard::rook_attacks(opp_player[Piece::King], total_occupied);
  }
  if (cur_player.can_castle_kingside() && ((total_occupied & (king << 1 | king << 2)) == 0) &&
      !is_under_attack(king << 1) && !is_under_attack(king << 2) && (rook_to_mask & king << 1)) {
    moves.emplace_back(Piece::King, king, king << 2);
  }
  if (cur_player.can_castle_queenside() && ((total_occupied & (king >> 1 | king >> 2 | king >> 3)) == 0) &&
      !is_under_attack(king >> 1) && !is_under_attack(king >> 2) && (rook_to_mask & king >> 1)) {
    moves.emplace_back(Piece::King, king, king >> 2);
  }
}

template <bool IsWhite>
template <typename MoveGen<IsWhite>::MoveType MT>
void MoveGen<IsWhite>::generate_unchecked_knight_moves(MoveContainer& moves) const {
  // Pinned knights cannot move, while un-pinned knights can move anywhere.
  u64 knights = cur_player[Piece::Knight] & ~pinned_pieces;
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::knight_attacks(opp_player[Piece::King]);
  }
  BITBOARD_ITERATE(knights, from) {
    u64 to_bitboard = bitboard::knight_attacks(from) & ~cur_occupied & to_mask;
    BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(Piece::Knight, from, to, get_opp_piece_at(to)); }
  }
}

template <bool IsWhite>
template <typename MoveGen<IsWhite>::MoveType MT>
void MoveGen<IsWhite>::generate_unchecked_pawn_moves(MoveContainer& moves) const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of the pinner.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) {
    constexpr u64 promotion_squares = IsWhite ? bitboard::RANK_8 : bitboard::RANK_1;
    to_mask = opp_occupied | promotion_squares;
  }
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    constexpr u64 promotion_squares = IsWhite ? bitboard::RANK_8 : bitboard::RANK_1;
    to_mask = bitboard::pawn_attacks<!IsWhite>(opp_player[Piece::King]) | opp_occupied | promotion_squares;
  }

  const u64 unpinned_pawns = cur_player[Piece::Pawn] & ~pinned_pieces;
  BITBOARD_ITERATE(unpinned_pawns, from) {
    const u64 capture_bitboard = bitboard::pawn_attacks<IsWhite>(from) & opp_occupied & to_mask;
    BITBOARD_ITERATE(capture_bitboard, to) { add_pawn_capture(moves, from, to); }
    const u64 push_bitboard = bitboard::pawn_pushes<IsWhite>(from, total_occupied) & to_mask;
    BITBOARD_ITERATE(push_bitboard, to) { add_pawn_push(moves, from, to); }
  }

  const u64 pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player[Piece::Pawn];
  BITBOARD_ITERATE(pinned_pawns, from) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      if ((pinned_by & bitboard::pawn_attacks<IsWhite>(from)) == 0) continue;
      add_pawn_capture(moves, from, pinned_by);
    } else {
      // Pinned along a rook ray.
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      u64 to_bitboard = bitboard::pawn_pushes<IsWhite>(from, total_occupied) & to_mask;
      to_bitboard &= bitboard::between(cur_player[Piece::King], pinned_by);
      BITBOARD_ITERATE(to_bitboard, to) { add_pawn_push(moves, from, to); }
    }
  }

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would be wrongly found to be legal.
  if (board.get_en_passant()) {
    const u64 from_bitboard = bitboard::pawn_attacks<!IsWhite>(board.get_en_passant()) & cur_player[Piece::Pawn];
    BITBOARD_ITERATE(from_bitboard, from) {
      const u64 captured_pawn = IsWhite ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
      if (bitboard::bishop_attacks(cur_player[Piece::King],
                                   total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
        continue;
      if (bitboard::rook_attacks(cur_player[Piece::King],
                                 total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
        continue;
      moves.emplace_back(Piece::Pawn, from, board.get_en_passant(), Piece::Pawn);
    }
  }
}

template <bool IsWhite>
template <typename MoveGen<IsWhite>::MoveType MT>
void MoveGen<IsWhite>::generate_unchecked_rooklike_moves(MoveContainer& moves) const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::rook_attacks(opp_player[Piece::King], total_occupied);
  }

  for (Piece from_piece : {Piece::Rook, Piece::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == Piece::Queen) to_mask |= bitboard::queen_attacks(opp_player[Piece::King], total_occupied);
    }
    const u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      const u64 to_bitboard = bitboard::rook_attacks(from, total_occupied) & ~cur_occupied & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }

    const u64 pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      const u64 to_bitboard = (bitboard::between(cur_player[Piece::King], pinned_by) | pinned_by) & ~from & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }
  }
}

template <bool IsWhite>
void MoveGen<IsWhite>::generate_king_single_check_evasions(MoveContainer& moves, u64 attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  // 1. Capture the attacker with a piece that is not pinned.
  const u64 attacker_bishop_rays = bitboard::bishop_attacks(attacker, total_occupied);
  const u64 attacker_rook_rays = bitboard::rook_attacks(attacker, total_occupied);
  const Piece attacker_piece = get_opp_piece_at(attacker);
  // Capture the attacker with a bishop that is not pinned.
  u64 bishop_capturers = attacker_bishop_rays & cur_player[Piece::Bishop] & ~pinned_pieces;
  BITBOARD_ITERATE(bishop_capturers, from) { moves.emplace_back(Piece::Bishop, from, attacker, attacker_piece); }
  // Capture the attacker with a knight that is not pinned.
  u64 knight_capturers = bitboard::knight_attacks(attacker) & cur_player[Piece::Knight] & ~pinned_pieces;
  BITBOARD_ITERATE(knight_capturers, from) { moves.emplace_back(Piece::Knight, from, attacker, attacker_piece); }
  // Capture the attacker with a pawn that is not pinned.
  u64 pawn_capturers = bitboard::pawn_attacks<!IsWhite>(attacker) & cur_player[Piece::Pawn] & ~pinned_pieces;
  BITBOARD_ITERATE(pawn_capturers, from) { add_pawn_capture(moves, from, attacker); }
  // Special en-passant check for pawns.
  if (attacker == (IsWhite ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    u64 capturing_square = IsWhite ? attacker << 8 : attacker >> 8;
    u64 pawn_capturers = bitboard::pawn_attacks<!IsWhite>(capturing_square) & cur_player[Piece::Pawn] & ~pinned_pieces;
    BITBOARD_ITERATE(pawn_capturers, from) { moves.emplace_back(Piece::Pawn, from, capturing_square, attacker_piece); }
  }
  // Capture the attacker with a queen that is not pinned.
  u64 queen_capturers = (attacker_bishop_rays | attacker_rook_rays) & cur_player[Piece::Queen] & ~pinned_pieces;
  BITBOARD_ITERATE(queen_capturers, from) { moves.emplace_back(Piece::Queen, from, attacker, attacker_piece); }
  // Capture the attacker with a rook that is not pinned.
  u64 rook_capturers = attacker_rook_rays & cur_player[Piece::Rook] & ~pinned_pieces;
  BITBOARD_ITERATE(rook_capturers, from) { moves.emplace_back(Piece::Rook, from, attacker, attacker_piece); }

  // 2. King moves to a square that is not attacked. This is the same as evading double check.
  generate_king_double_check_evasions<MoveType::All>(moves);

  // 3. Block the check if it is a sliding check.
  if (piece::is_slider(attacker_piece)) {
    u64 blocking_squares = bitboard::between(cur_player[Piece::King], attacker);
    BITBOARD_ITERATE(blocking_squares, to) {
      const u64 blocker_bishop_rays = bitboard::bishop_attacks(to, total_occupied);
      const u64 blocker_rook_rays = bitboard::rook_attacks(to, total_occupied);
      // Block with a bishop that is not pinned.
      u64 bishop_blockers = blocker_bishop_rays & cur_player[Piece::Bishop] & ~pinned_pieces;
      BITBOARD_ITERATE(bishop_blockers, from) { moves.emplace_back(Piece::Bishop, from, to); }
      // Block with a knight that is not pinned.
      u64 knight_blockers = bitboard::knight_attacks(to) & cur_player[Piece::Knight] & ~pinned_pieces;
      BITBOARD_ITERATE(knight_blockers, from) { moves.emplace_back(Piece::Knight, from, to); }
      // Block with a pawn that is not pinned.
      u64 pawn_blockers = (IsWhite ? to >> 8 : to << 8);
      if constexpr (IsWhite) {
        pawn_blockers |= ((to >> 8) & ~total_occupied) >> 8 & bitboard::RANK_2;
      } else {
        pawn_blockers |= ((to << 8) & ~total_occupied) << 8 & bitboard::RANK_7;
      }
      pawn_blockers = pawn_blockers & cur_player[Piece::Pawn] & ~pinned_pieces;
      BITBOARD_ITERATE(pawn_blockers, from) { moves.emplace_back(Piece::Pawn, from, to); }
      // Block with a queen that is not pinned.
      u64 queen_blockers = (blocker_bishop_rays | blocker_rook_rays) & cur_player[Piece::Queen] & ~pinned_pieces;
      BITBOARD_ITERATE(queen_blockers, from) { moves.emplace_back(Piece::Queen, from, to); }
      // Block with a rook that is not pinned.
      u64 rook_blockers = blocker_rook_rays & cur_player[Piece::Rook] & ~pinned_pieces;
      BITBOARD_ITERATE(rook_blockers, from) { moves.emplace_back(Piece::Rook, from, to); }
    }
  }
}

template <bool IsWhite>
template <typename MoveGen<IsWhite>::MoveType MT>
void MoveGen<IsWhite>::generate_king_double_check_evasions(MoveContainer& moves) const {
  // To evade a double check, the king must move to a square that is not attacked.
  const u64 king = cur_player[Piece::King];
  u64 to_bitboard = bitboard::king_attacks(king) & ~cur_occupied &
                    ~bitboard::pawn_attacks<!IsWhite>(opp_player[Piece::Pawn]) &
                    ~bitboard::king_attacks(opp_player[Piece::King]);
  if constexpr (MT != MoveType::All) to_bitboard &= opp_occupied;
  const u64 total_occupied_without_king = total_occupied ^ king;
  BITBOARD_ITERATE(to_bitboard, to) {
    if (bitboard::knight_attacks(to) & opp_player[Piece::Knight]) continue;
    if (bitboard::bishop_attacks(to, total_occupied_without_king) &
        (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
      continue;
    if (bitboard::rook_attacks(to, total_occupied_without_king) & (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
      continue;
    moves.emplace_back(Piece::King, king, to, get_opp_piece_at(to));
  }
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_unchecked_bishoplike_moves(u64 pinned_pieces) const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  for (Piece from_piece : {Piece::Bishop, Piece::Queen}) {
    const u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      const u64 to_bitboard = bitboard::bishop_attacks(from, total_occupied) & ~cur_occupied;
      if (to_bitboard) return true;
    }

    const u64 pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      const u64 to_bitboard = (bitboard::between(cur_player[Piece::King], pinned_by) | pinned_by) & ~from;
      if (to_bitboard) return true;
    }
  }

  return false;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_unchecked_king_moves() const {
  // The king is not in check, hence it either moves to an unattacked square, or we may castle.
  if (has_king_double_check_evasions()) return true;

  const u64 king = cur_player[Piece::King];
  if (cur_player.can_castle_kingside() && ((total_occupied & (king << 1 | king << 2)) == 0) &&
      !is_under_attack(king << 1) && !is_under_attack(king << 2)) {
    return true;
  }
  if (cur_player.can_castle_queenside() && ((total_occupied & (king >> 1 | king >> 2 | king >> 3)) == 0) &&
      !is_under_attack(king >> 1) && !is_under_attack(king >> 2)) {
    return true;
  }

  return false;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_unchecked_knight_moves(u64 pinned_pieces) const {
  // Pinned knights cannot move, while un-pinned knights can move anywhere.
  u64 knights = cur_player[Piece::Knight] & ~pinned_pieces;
  BITBOARD_ITERATE(knights, from) {
    u64 to_bitboard = bitboard::knight_attacks(from) & ~cur_occupied;
    if (to_bitboard) return true;
  }
  return false;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_unchecked_pawn_moves(u64 pinned_pieces) const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of the pinner.
  const u64 unpinned_pawns = cur_player[Piece::Pawn] & ~pinned_pieces;
  BITBOARD_ITERATE(unpinned_pawns, from) {
    const u64 capture_bitboard = bitboard::pawn_attacks<IsWhite>(from) & opp_occupied;
    const u64 push_bitboard = bitboard::pawn_pushes<IsWhite>(from, total_occupied);
    if (push_bitboard | capture_bitboard) return true;
  }

  const u64 pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player[Piece::Pawn];
  BITBOARD_ITERATE(pinned_pawns, from) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      if (pinned_by & bitboard::pawn_attacks<IsWhite>(from)) return true;
    } else {
      // Pinned along a rook ray.
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      u64 to_bitboard = bitboard::pawn_pushes<IsWhite>(from, total_occupied);
      to_bitboard &= bitboard::between(cur_player[Piece::King], pinned_by);
      if (to_bitboard) return true;
    }
  }

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would be wrongly found to be legal.
  if (board.get_en_passant()) {
    const u64 from_bitboard = bitboard::pawn_attacks<!IsWhite>(board.get_en_passant()) & cur_player[Piece::Pawn];
    BITBOARD_ITERATE(from_bitboard, from) {
      const u64 captured_pawn = IsWhite ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
      if (bitboard::bishop_attacks(cur_player[Piece::King],
                                   total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
        continue;
      if (bitboard::rook_attacks(cur_player[Piece::King],
                                 total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
        continue;
      return true;
    }
  }

  return false;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_unchecked_rooklike_moves(u64 pinned_pieces) const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  for (Piece from_piece : {Piece::Rook, Piece::Queen}) {
    const u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      const u64 to_bitboard = bitboard::rook_attacks(from, total_occupied) & ~cur_occupied;
      if (to_bitboard) return true;
    }

    const u64 pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::beyond(cur_player[Piece::King], from) & pinners;
      const u64 to_bitboard = (bitboard::between(cur_player[Piece::King], pinned_by) | pinned_by) & ~from;
      if (to_bitboard) return true;
    }
  }

  return false;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_king_single_check_evasions(u64 attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  // 1. Capture the attacker with a piece that is not pinned.
  const u64 attacker_bishop_rays = bitboard::bishop_attacks(attacker, total_occupied);
  const u64 attacker_rook_rays = bitboard::rook_attacks(attacker, total_occupied);
  // Capture the attacker with a bishop that is not pinned.
  u64 bishop_capturers = attacker_bishop_rays & cur_player[Piece::Bishop] & ~pinned_pieces;
  if (bishop_capturers) return true;
  // Capture the attacker with a knight that is not pinned.
  u64 knight_capturers = bitboard::knight_attacks(attacker) & cur_player[Piece::Knight] & ~pinned_pieces;
  if (knight_capturers) return true;
  // Capture the attacker with a pawn that is not pinned.
  u64 pawn_capturers = bitboard::pawn_attacks<!IsWhite>(attacker) & cur_player[Piece::Pawn] & ~pinned_pieces;
  if (pawn_capturers) return true;
  // Special en-passant check for pawns.
  if (attacker == (IsWhite ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    u64 capturing_square = IsWhite ? attacker << 8 : attacker >> 8;
    u64 pawn_capturers = bitboard::pawn_attacks<!IsWhite>(capturing_square) & cur_player[Piece::Pawn] & ~pinned_pieces;
    if (pawn_capturers) return true;
  }
  // Capture the attacker with a queen that is not pinned.
  u64 queen_capturers = (attacker_bishop_rays | attacker_rook_rays) & cur_player[Piece::Queen] & ~pinned_pieces;
  if (queen_capturers) return true;
  // Capture the attacker with a rook that is not pinned.
  u64 rook_capturers = attacker_rook_rays & cur_player[Piece::Rook] & ~pinned_pieces;
  if (rook_capturers) return true;

  // 2. King moves to a square that is not attacked. This is the same as evading double check.
  if (has_king_double_check_evasions()) return true;

  // 3. Block the check if it is a sliding check.
  const Piece attacker_piece = get_opp_piece_at(attacker);
  if (piece::is_slider(attacker_piece)) {
    u64 blocking_squares = bitboard::between(cur_player[Piece::King], attacker);
    BITBOARD_ITERATE(blocking_squares, to) {
      const u64 blocker_bishop_rays = bitboard::bishop_attacks(to, total_occupied);
      const u64 blocker_rook_rays = bitboard::rook_attacks(to, total_occupied);
      // Block with a bishop that is not pinned.
      u64 bishop_blockers = blocker_bishop_rays & cur_player[Piece::Bishop] & ~pinned_pieces;
      if (bishop_blockers) return true;
      // Block with a knight that is not pinned.
      u64 knight_blockers = bitboard::knight_attacks(to) & cur_player[Piece::Knight] & ~pinned_pieces;
      if (knight_blockers) return true;
      // Block with a pawn that is not pinned.
      u64 pawn_blockers = (IsWhite ? to >> 8 : to << 8);
      if constexpr (IsWhite) {
        pawn_blockers |= ((to >> 8) & ~total_occupied) >> 8 & bitboard::RANK_2;
      } else {
        pawn_blockers |= ((to << 8) & ~total_occupied) << 8 & bitboard::RANK_7;
      }
      pawn_blockers = pawn_blockers & cur_player[Piece::Pawn] & ~pinned_pieces;
      if (pawn_blockers) return true;
      // Block with a queen that is not pinned.
      u64 queen_blockers = (blocker_bishop_rays | blocker_rook_rays) & cur_player[Piece::Queen] & ~pinned_pieces;
      if (queen_blockers) return true;
      // Block with a rook that is not pinned.
      u64 rook_blockers = blocker_rook_rays & cur_player[Piece::Rook] & ~pinned_pieces;
      if (rook_blockers) return true;
    }
  }

  return false;
}

template <bool IsWhite>
bool MoveGen<IsWhite>::has_king_double_check_evasions() const {
  // To evade a double check, the king must move to a square that is not attacked.
  const u64 king = cur_player[Piece::King];
  const u64 to_bitboard = bitboard::king_attacks(king) & ~cur_occupied &
                          ~bitboard::pawn_attacks<!IsWhite>(opp_player[Piece::Pawn]) &
                          ~bitboard::king_attacks(opp_player[Piece::King]);
  const u64 total_occupied_without_king = total_occupied ^ king;
  BITBOARD_ITERATE(to_bitboard, to) {
    if (bitboard::knight_attacks(to) & opp_player[Piece::Knight]) continue;
    if (bitboard::bishop_attacks(to, total_occupied_without_king) &
        (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
      continue;
    if (bitboard::rook_attacks(to, total_occupied_without_king) & (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
      continue;
    return true;
  }
  return false;
}

MoveContainer move_gen::generate_moves(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<true>{board}.generate_moves();
  } else {
    return MoveGen<false>{board}.generate_moves();
  }
}

MoveContainer move_gen::generate_quiescence_moves(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<true>{board}.generate_quiescence_moves();
  } else {
    return MoveGen<false>{board}.generate_quiescence_moves();
  }
}

MoveContainer move_gen::generate_quiescence_moves_and_checks(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<true>{board}.generate_quiescence_moves_and_checks();
  } else {
    return MoveGen<false>{board}.generate_quiescence_moves_and_checks();
  }
}

bool move_gen::has_moves(const Board& board) {
  if (board.is_white_to_move()) {
    return MoveGen<true>{board}.has_moves();
  } else {
    return MoveGen<false>{board}.has_moves();
  }
}

bool move_gen::is_under_attack(const Board& board, u64 square) {
  if (board.is_white_to_move()) {
    return MoveGen<true>{board}.is_under_attack(square);
  } else {
    return MoveGen<false>{board}.is_under_attack(square);
  }
}
