#include "move_gen.h"

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

MoveGen::MoveGen(const Board& board)
    : board{board},
      cur_player{board.cur_player()},
      opp_player{board.opp_player()},
      cur_occupied{cur_player.occupied()},
      opp_occupied{opp_player.occupied()},
      total_occupied{cur_occupied | opp_occupied},
      opp_piece_at{compute_piece_at(opp_player)},
      king_bishop_rays{bitboard::bishop_attacks(cur_player[Piece::King], total_occupied)},
      king_rook_rays{bitboard::rook_attacks(cur_player[Piece::King], total_occupied)},
      pinned_pieces{get_pinned_pieces()} {}

MoveContainer MoveGen::generate_quiescence_moves() const {
  MoveContainer moves;
  generate_unchecked_bishoplike_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_king_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_pawn_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_rooklike_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  return moves;
}

MoveContainer MoveGen::generate_quiescence_moves_and_checks() const {
  MoveContainer moves;
  generate_unchecked_bishoplike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_king_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_pawn_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_rooklike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);

  // The move generation above can only generate checks where the piece moved is the one giving check.
  // Another type of check is where the piece moves and another piece is the one giving check, which we generate below.
  const u64 opp_king_bishop_rays = bitboard::bishop_attacks(opp_player[Piece::King], total_occupied);
  const u64 opp_king_rook_rays = bitboard::rook_attacks(opp_player[Piece::King], total_occupied);
  const u64 cur_king_attackers = get_king_attackers();

  // The piece at `from`, if moved to anywhere in `to_mask`, will cause a check by another piece.
  // Note that captures and promotions are ignored, as they are generated above already.
  auto generate_indirect_checks = [&](const u64 from, u64 to_mask) {
    const Piece piece = cur_player.piece_at(from);
    if (pinned_pieces & from) {
      // The piece is pinned to its own king, can only move along the pin ray.
      const u64 new_king_attackers = bitboard::queen_attacks(cur_player[Piece::King], total_occupied ^ from);
      const u64 pinner = new_king_attackers ^ cur_king_attackers;
      to_mask &= bitboard::block_slider_check(cur_player[Piece::King], pinner);
    }
    to_mask &= ~cur_occupied & ~opp_occupied;
    switch (piece) {
      case Piece::Bishop:
        to_mask &= ~opp_king_bishop_rays & bitboard::bishop_attacks(from, total_occupied);
        break;
      case Piece::King:
        // TODO:
        to_mask &= bitboard::king_attacks(from);
        break;
      case Piece::Knight:
        to_mask &= ~bitboard::knight_attacks(opp_player[Piece::King]) & bitboard::knight_attacks(from);
        break;
      case Piece::Pawn:
        to_mask &= ~bitboard::pawn_attacks(opp_player[Piece::King], !board.is_white_to_move()) & ~bitboard::RANK_1 &
                   ~bitboard::RANK_8 & bitboard::pawn_pushes(from, total_occupied, board.is_white_to_move());
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
        if (bitboard::pawn_attacks(to, board.is_white_to_move()) & opp_player[Piece::Pawn]) continue;
        if (bitboard::rook_attacks(to, total_occupied) & (opp_player[Piece::Rook] | opp_player[Piece::Queen])) continue;
        moves.emplace_back(piece, from, to, get_opp_piece_at(to));
      }
    } else {
      BITBOARD_ITERATE(to_mask, to) { moves.emplace_back(piece, from, to, get_opp_piece_at(to)); }
    }
  };

  u64 maybe_self_pinned_by_rooks = opp_king_rook_rays & cur_occupied;
  const u64 rook_attackers = opp_king_rook_rays & (cur_player[Piece::Rook] | cur_player[Piece::Queen]);
  BITBOARD_ITERATE(maybe_self_pinned_by_rooks, from) {
    const u64 new_rook_rays = bitboard::rook_attacks(opp_player[Piece::King], total_occupied ^ from);
    const u64 new_rook_attackers = new_rook_rays & (cur_player[Piece::Rook] | cur_player[Piece::Queen]);
    if (new_rook_attackers == rook_attackers) continue;
    u64 to_mask = ~bitboard::block_slider_check(opp_player[Piece::King], new_rook_attackers ^ rook_attackers);
    generate_indirect_checks(from, to_mask);
  }
  u64 maybe_self_pinned_by_bishops = opp_king_bishop_rays & cur_occupied;
  const u64 bishop_attackers = opp_king_bishop_rays & (cur_player[Piece::Bishop] | cur_player[Piece::Queen]);
  BITBOARD_ITERATE(maybe_self_pinned_by_bishops, from) {
    const u64 new_bishop_rays = bitboard::bishop_attacks(opp_player[Piece::King], total_occupied ^ from);
    const u64 new_bishop_attackers = new_bishop_rays & (cur_player[Piece::Bishop] | cur_player[Piece::Queen]);
    if (new_bishop_attackers == bishop_attackers) continue;
    u64 to_mask = ~bitboard::block_slider_check(opp_player[Piece::King], new_bishop_attackers ^ bishop_attackers);
    generate_indirect_checks(from, to_mask);
  }

  return moves;
}

MoveContainer MoveGen::generate_moves() const {
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

bool MoveGen::has_moves() const {
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

bool MoveGen::is_under_attack(u64 square) const {
  if (bitboard::bishop_attacks(square, total_occupied) & (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) {
    return true;
  }
  if (bitboard::king_attacks(square) & opp_player[Piece::King]) return true;
  if (bitboard::knight_attacks(square) & opp_player[Piece::Knight]) return true;
  if (bitboard::pawn_attacks(square, board.is_white_to_move()) & opp_player[Piece::Pawn]) return true;
  if (bitboard::rook_attacks(square, total_occupied) & (opp_player[Piece::Rook] | opp_player[Piece::Queen])) {
    return true;
  }
  return false;
}

void MoveGen::add_pawn_moves(MoveContainer& moves, u64 from, u64 to) const {
  Piece captured = board.get_en_passant() == to ? Piece::Pawn : get_opp_piece_at(to);
  if (to & (bitboard::RANK_1 | bitboard::RANK_8)) {
    moves.emplace_back(from, to, Piece::Bishop, captured);
    moves.emplace_back(from, to, Piece::Knight, captured);
    moves.emplace_back(from, to, Piece::Queen, captured);
    moves.emplace_back(from, to, Piece::Rook, captured);
  } else {
    moves.emplace_back(Piece::Pawn, from, to, captured);
  }
}

u64 MoveGen::get_king_attackers() const {
  u64 king = cur_player[Piece::King];
  u64 attackers = (king_bishop_rays & (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) |
                  (bitboard::knight_attacks(king) & opp_player[Piece::Knight]) |
                  (bitboard::pawn_attacks(king, board.is_white_to_move()) & opp_player[Piece::Pawn]) |
                  (king_rook_rays & (opp_player[Piece::Rook] | opp_player[Piece::Queen]));
  return attackers;
}

u64 MoveGen::get_pinned_pieces() const {
  const u64 king = cur_player[Piece::King];
  const u64 current_slider_attackers = (king_bishop_rays & (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) |
                                       (king_rook_rays & (opp_player[Piece::Rook] | opp_player[Piece::Queen]));
  const u64 potential_pinned_pieces = (king_bishop_rays | king_rook_rays) & cur_occupied;
  const u64 new_slider_attackers = (bitboard::bishop_attacks(king, total_occupied ^ potential_pinned_pieces) &
                                    (opp_player[Piece::Bishop] | opp_player[Piece::Queen])) |
                                   (bitboard::rook_attacks(king, total_occupied ^ potential_pinned_pieces) &
                                    (opp_player[Piece::Rook] | opp_player[Piece::Queen]));
  u64 pinners = new_slider_attackers ^ current_slider_attackers;
  u64 pinned_pieces = 0;
  BITBOARD_ITERATE(pinners, pinner) {
    const u64 pinned_piece = bitboard::block_slider_check(king, pinner) & cur_occupied;
    pinned_pieces ^= pinned_piece;
  }
  return pinned_pieces;
}

Piece MoveGen::get_opp_piece_at(u64 bit) const {
  int index = bit::to_index(bit);
  int piece =
      (opp_piece_at[0] >> index & 1) + ((opp_piece_at[1] >> index & 1) << 1) + ((opp_piece_at[2] >> index & 1) << 2);
  return static_cast<Piece>(piece);
}

template <MoveGen::MoveType MT>
void MoveGen::generate_unchecked_bishoplike_moves(MoveContainer& moves) const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  const u64 opp_blockers = king_bishop_rays & opp_occupied;  // Opponent pieces that block bishop rays from the king.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::bishop_attacks(opp_player[Piece::King], total_occupied);
  }

  for (Piece from_piece : {Piece::Bishop, Piece::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == Piece::Queen) to_mask |= bitboard::queen_attacks(opp_player[Piece::King], total_occupied);
    }
    u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      u64 to_bitboard = bitboard::bishop_attacks(from, total_occupied) & ~cur_occupied & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }

    u64 pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::bishop_attacks(cur_player[Piece::King], total_occupied ^ from) &
                            (opp_player[Piece::Bishop] | opp_player[Piece::Queen]) & ~opp_blockers;
      u64 to_bitboard =
          (bitboard::block_slider_check(cur_player[Piece::King], pinned_by) | pinned_by) & ~from & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }
  }
}

template <MoveGen::MoveType MT>
void MoveGen::generate_unchecked_king_moves(MoveContainer& moves) const {
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

template <MoveGen::MoveType MT>
void MoveGen::generate_unchecked_knight_moves(MoveContainer& moves) const {
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

template <MoveGen::MoveType MT>
void MoveGen::generate_unchecked_pawn_moves(MoveContainer& moves) const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of the pinner.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) {
    to_mask = opp_occupied | (board.is_white_to_move() ? bitboard::RANK_8 : bitboard::RANK_1);
  }
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = bitboard::pawn_attacks(opp_player[Piece::King], !board.is_white_to_move()) | opp_occupied |
              (board.is_white_to_move() ? bitboard::RANK_8 : bitboard::RANK_1);
  }

  u64 unpinned_pawns = cur_player[Piece::Pawn] & ~pinned_pieces;
  BITBOARD_ITERATE(unpinned_pawns, from) {
    u64 to_bitboard = ((bitboard::pawn_attacks(from, board.is_white_to_move()) & opp_occupied) |
                       bitboard::pawn_pushes(from, total_occupied, board.is_white_to_move())) &
                      to_mask;
    BITBOARD_ITERATE(to_bitboard, to) { add_pawn_moves(moves, from, to); }
  }

  const u64 bishop_ray_blockers = king_bishop_rays & opp_occupied;
  const u64 rook_ray_blockers = king_rook_rays & opp_occupied;
  u64 pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player[Piece::Pawn];
  BITBOARD_ITERATE(pinned_pawns, from) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const u64 pinned_by = bitboard::bishop_attacks(cur_player[Piece::King], total_occupied ^ from) &
                            (opp_player[Piece::Bishop] | opp_player[Piece::Queen]) & ~bishop_ray_blockers;
      if (pinned_by == 0) continue;
      if ((pinned_by & bitboard::pawn_attacks(from, board.is_white_to_move())) == 0) continue;
      add_pawn_moves(moves, from, pinned_by);
    } else {
      // Pinned along a rook ray.
      u64 to_bitboard = bitboard::pawn_pushes(from, total_occupied, board.is_white_to_move()) & to_mask;
      const u64 rook = bitboard::rook_attacks(cur_player[Piece::King], total_occupied ^ from) &
                       (opp_player[Piece::Rook] | opp_player[Piece::Queen]) & ~rook_ray_blockers;
      to_bitboard &= bitboard::block_slider_check(cur_player[Piece::King], rook);
      BITBOARD_ITERATE(to_bitboard, to) { add_pawn_moves(moves, from, to); }
    }
  }

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would be wrongly found to be legal.
  if (board.get_en_passant()) {
    u64 from_bitboard =
        bitboard::pawn_attacks(board.get_en_passant(), !board.is_white_to_move()) & cur_player[Piece::Pawn];
    BITBOARD_ITERATE(from_bitboard, from) {
      const u64 captured_pawn = board.is_white_to_move() ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
      if (bitboard::bishop_attacks(cur_player[Piece::King],
                                   total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
        continue;
      if (bitboard::rook_attacks(cur_player[Piece::King],
                                 total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()) &
          (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
        continue;
      add_pawn_moves(moves, from, board.get_en_passant());
    }
  }
}

template <MoveGen::MoveType MT>
void MoveGen::generate_unchecked_rooklike_moves(MoveContainer& moves) const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  const u64 opp_blockers = king_rook_rays & opp_occupied;  // Opponent pieces that block bishop rays from the king.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::rook_attacks(opp_player[Piece::King], total_occupied);
  }

  for (Piece from_piece : {Piece::Rook, Piece::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == Piece::Queen) to_mask |= bitboard::queen_attacks(opp_player[Piece::King], total_occupied);
    }
    u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      u64 to_bitboard = bitboard::rook_attacks(from, total_occupied) & ~cur_occupied & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }

    u64 pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::rook_attacks(cur_player[Piece::King], total_occupied ^ from) &
                            (opp_player[Piece::Rook] | opp_player[Piece::Queen]) & ~opp_blockers;
      u64 to_bitboard =
          (bitboard::block_slider_check(cur_player[Piece::King], pinned_by) | pinned_by) & ~from & to_mask;
      BITBOARD_ITERATE(to_bitboard, to) { moves.emplace_back(from_piece, from, to, get_opp_piece_at(to)); }
    }
  }
}

void MoveGen::generate_king_single_check_evasions(MoveContainer& moves, u64 attacker) const {
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
  u64 pawn_capturers =
      bitboard::pawn_attacks(attacker, !board.is_white_to_move()) & cur_player[Piece::Pawn] & ~pinned_pieces;
  BITBOARD_ITERATE(pawn_capturers, from) { add_pawn_moves(moves, from, attacker); }
  // Special en-passant check for pawns.
  if (attacker == (board.is_white_to_move() ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    u64 capturing_square = board.is_white_to_move() ? attacker << 8 : attacker >> 8;
    u64 pawn_capturers =
        bitboard::pawn_attacks(capturing_square, !board.is_white_to_move()) & cur_player[Piece::Pawn] & ~pinned_pieces;
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
    u64 blocking_squares = bitboard::block_slider_check(cur_player[Piece::King], attacker);
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
      u64 pawn_blockers = (board.is_white_to_move() ? to >> 8 : to << 8);
      if (board.is_white_to_move()) {
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

template <MoveGen::MoveType MT>
void MoveGen::generate_king_double_check_evasions(MoveContainer& moves) const {
  // To evade a double check, the king must move to a square that is not attacked.
  const u64 king = cur_player[Piece::King];
  u64 to_bitboard = bitboard::king_attacks(king) & ~cur_occupied;
  if constexpr (MT != MoveType::All) to_bitboard &= opp_occupied;
  const u64 total_occupied_without_king = (cur_occupied | opp_occupied) ^ king;
  BITBOARD_ITERATE(to_bitboard, to) {
    if (bitboard::bishop_attacks(to, total_occupied_without_king) &
        (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
      continue;
    if (bitboard::king_attacks(to) & opp_player[Piece::King]) continue;
    if (bitboard::knight_attacks(to) & opp_player[Piece::Knight]) continue;
    if (bitboard::pawn_attacks(to, board.is_white_to_move()) & opp_player[Piece::Pawn]) continue;
    if (bitboard::rook_attacks(to, total_occupied_without_king) & (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
      continue;
    moves.emplace_back(Piece::King, king, to, get_opp_piece_at(to));
  }
}

bool MoveGen::has_unchecked_bishoplike_moves(u64 pinned_pieces) const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  const u64 opp_blockers = king_bishop_rays & opp_occupied;  // Opponent pieces that block bishop rays from the king.

  for (Piece from_piece : {Piece::Bishop, Piece::Queen}) {
    u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      u64 to_bitboard = bitboard::bishop_attacks(from, total_occupied) & ~cur_occupied;
      if (to_bitboard) return true;
    }

    u64 pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::bishop_attacks(cur_player[Piece::King], total_occupied ^ from) &
                            (opp_player[Piece::Bishop] | opp_player[Piece::Queen]) & ~opp_blockers;
      u64 to_bitboard = (bitboard::block_slider_check(cur_player[Piece::King], pinned_by) | pinned_by) & ~from;
      if (to_bitboard) return true;
    }
  }

  return false;
}

bool MoveGen::has_unchecked_king_moves() const {
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

bool MoveGen::has_unchecked_knight_moves(u64 pinned_pieces) const {
  // Pinned knights cannot move, while un-pinned knights can move anywhere.
  u64 knights = cur_player[Piece::Knight] & ~pinned_pieces;
  BITBOARD_ITERATE(knights, from) {
    u64 to_bitboard = bitboard::knight_attacks(from) & ~cur_occupied;
    if (to_bitboard) return true;
  }
  return false;
}

bool MoveGen::has_unchecked_pawn_moves(u64 pinned_pieces) const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of the pinner.
  u64 unpinned_pawns = cur_player[Piece::Pawn] & ~pinned_pieces;
  BITBOARD_ITERATE(unpinned_pawns, from) {
    u64 to_bitboard = bitboard::pawn_attacks(from, board.is_white_to_move()) & opp_occupied;
    if (board.is_white_to_move()) {
      to_bitboard |=
          (from << 8 & ~total_occupied) | ((from & bitboard::RANK_2) << 16 & ~total_occupied & ~(total_occupied << 8));
    } else {
      to_bitboard |=
          (from >> 8 & ~total_occupied) | ((from & bitboard::RANK_7) >> 16 & ~total_occupied & ~(total_occupied >> 8));
    }
    if (to_bitboard) return true;
  }

  const u64 bishop_ray_blockers = king_bishop_rays & opp_occupied;
  const u64 rook_ray_blockers = king_rook_rays & opp_occupied;
  u64 pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player[Piece::Pawn];
  BITBOARD_ITERATE(pinned_pawns, from) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const u64 pinned_by = bitboard::bishop_attacks(cur_player[Piece::King], total_occupied ^ from) &
                            (opp_player[Piece::Bishop] | opp_player[Piece::Queen]) & ~bishop_ray_blockers;
      if (pinned_by == 0) continue;
      if (pinned_by & bitboard::pawn_attacks(from, board.is_white_to_move())) return true;
    } else {
      // Pinned along a rook ray.
      u64 to_bitboard = bitboard::pawn_pushes(from, total_occupied, board.is_white_to_move());
      const u64 rook = bitboard::rook_attacks(cur_player[Piece::King], total_occupied ^ from) &
                       (opp_player[Piece::Rook] | opp_player[Piece::Queen]) & ~rook_ray_blockers;
      to_bitboard &= bitboard::block_slider_check(cur_player[Piece::King], rook);
      if (to_bitboard) return true;
    }
  }

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would be wrongly found to be legal.
  if (board.get_en_passant()) {
    u64 from_bitboard =
        bitboard::pawn_attacks(board.get_en_passant(), !board.is_white_to_move()) & cur_player[Piece::Pawn];
    BITBOARD_ITERATE(from_bitboard, from) {
      const u64 captured_pawn = board.is_white_to_move() ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
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

bool MoveGen::has_unchecked_rooklike_moves(u64 pinned_pieces) const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  const u64 opp_blockers = king_rook_rays & opp_occupied;  // Opponent pieces that block bishop rays from the king.

  for (Piece from_piece : {Piece::Rook, Piece::Queen}) {
    u64 unpinned_pieces = cur_player[from_piece] & ~pinned_pieces;
    BITBOARD_ITERATE(unpinned_pieces, from) {
      u64 to_bitboard = bitboard::rook_attacks(from, total_occupied) & ~cur_occupied;
      if (to_bitboard) return true;
    }

    u64 pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player[from_piece];
    BITBOARD_ITERATE(pinned_from_pieces, from) {
      const u64 pinned_by = bitboard::rook_attacks(cur_player[Piece::King], total_occupied ^ from) &
                            (opp_player[Piece::Rook] | opp_player[Piece::Queen]) & ~opp_blockers;
      u64 to_bitboard = (bitboard::block_slider_check(cur_player[Piece::King], pinned_by) | pinned_by) & ~from;
      if (to_bitboard) return true;
    }
  }

  return false;
}

bool MoveGen::has_king_single_check_evasions(u64 attacker) const {
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
  u64 pawn_capturers =
      bitboard::pawn_attacks(attacker, !board.is_white_to_move()) & cur_player[Piece::Pawn] & ~pinned_pieces;
  if (pawn_capturers) return true;
  // Special en-passant check for pawns.
  if (attacker == (board.is_white_to_move() ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    u64 capturing_square = board.is_white_to_move() ? attacker << 8 : attacker >> 8;
    u64 pawn_capturers =
        bitboard::pawn_attacks(capturing_square, !board.is_white_to_move()) & cur_player[Piece::Pawn] & ~pinned_pieces;
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
    u64 blocking_squares = bitboard::block_slider_check(cur_player[Piece::King], attacker);
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
      u64 pawn_blockers = (board.is_white_to_move() ? to >> 8 : to << 8);
      if (board.is_white_to_move()) {
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

bool MoveGen::has_king_double_check_evasions() const {
  // To evade a double check, the king must move to a square that is not attacked.
  const u64 king = cur_player[Piece::King];
  u64 to_bitboard = bitboard::king_attacks(king) & ~cur_occupied;
  const u64 total_occupied_without_king = (cur_occupied | opp_occupied) ^ king;
  BITBOARD_ITERATE(to_bitboard, to) {
    if (bitboard::bishop_attacks(to, total_occupied_without_king) &
        (opp_player[Piece::Bishop] | opp_player[Piece::Queen]))
      continue;
    if (bitboard::king_attacks(to) & opp_player[Piece::King]) continue;
    if (bitboard::knight_attacks(to) & opp_player[Piece::Knight]) continue;
    if (bitboard::pawn_attacks(to, board.is_white_to_move()) & opp_player[Piece::Pawn]) continue;
    if (bitboard::rook_attacks(to, total_occupied_without_king) & (opp_player[Piece::Rook] | opp_player[Piece::Queen]))
      continue;
    return true;
  }
  return false;
}