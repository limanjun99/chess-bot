#include "move_gen.h"

#include <array>

#include "bitboard.h"
#include "piece.h"
#include "pieces/base_piece.h"

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

  // Returns a bitboard of opponent pieces that attack the king.
  Bitboard get_king_attackers() const;

  // Gets the opponent piece at the given square.
  PieceType get_opp_piece_at(Bitboard bit) const;

  // Returns a bitboard of squares that a piece on the given square can move to (not accounting for legality).
  template <PieceType PT>
  Bitboard get_piece_moves(Bitboard square) const;

  // Returns a bitboard of squares that a piece on the given square can attack.
  template <PieceType PT>
  Bitboard get_piece_attacks(Bitboard square) const;

  // Returns a bitboard of squares that a opponent piece on the given square can attack.
  template <PieceType PT>
  Bitboard get_opp_piece_attacks(Bitboard square) const;

  // Adds a move to the container, accounting for promotions and captures.
  template <PieceType PT>
  void add_move(MoveContainer& moves, Bitboard from, Bitboard to) const;

  enum class MoveType { All, CapturesAndPromotionsOnly, CapturesChecksAndPromotionsOnly };

  // Generate legal moves of the piece, given that the king is not in check.
  template <PieceType PT, MoveType MT>
  void generate_unchecked_piece_moves(MoveContainer& moves) const;

  // Generate legal king moves given that the king is not in check.
  template <MoveType MT>
  void generate_unchecked_king_moves(MoveContainer& moves) const;

  // Generate legal king moves that escape the single-check.
  void generate_king_single_check_evasions(MoveContainer& moves, Bitboard attacker) const;

  // Generate legal king moves that escape the double-check.
  template <MoveType MT>
  void generate_king_double_check_evasions(MoveContainer& moves) const;

  // Check if there are legal moves for this piece given that the king is not in check.
  template <PieceType PT>
  bool has_unchecked_piece_moves() const;

  // Check if there are legal king moves given that the king is not in check.
  bool has_unchecked_king_moves() const;

  // Check if there are legal king moves that escape the single-check.
  bool has_king_single_check_evasions(Bitboard attacker) const;

  // Check if there are legal king moves that escape the double-check.
  bool has_king_double_check_evasions() const;
};

std::array<Bitboard, 3> compute_piece_at(const Player& player) {
  std::array<Bitboard, 3> piece_at{};
  // All the piece bitboards are compressed into 3 bitboards, where piece_at[i] represents whether the ith bit is set
  // for the piece at each square.
  const Bitboard none_bitboard = Bitboard::full & ~(player.occupied());
  piece_at[0] =
      player[static_cast<PieceType>(1)] | player[static_cast<PieceType>(3)] | player[static_cast<PieceType>(5)];
  piece_at[1] = player[static_cast<PieceType>(2)] | player[static_cast<PieceType>(3)] | none_bitboard;
  piece_at[2] = player[static_cast<PieceType>(4)] | player[static_cast<PieceType>(5)] | none_bitboard;
  return piece_at;
}

template <Color PlayerColor>
MoveGen<PlayerColor>::MoveGen(const Board& board)
    : board{board},
      cur_player{board.get_player<PlayerColor>()},
      opp_player{board.get_player<PlayerColor.flip()>()},
      cur_occupied{cur_player.occupied()},
      opp_occupied{opp_player.occupied()},
      total_occupied{cur_occupied | opp_occupied},
      opp_piece_at{compute_piece_at(opp_player)},
      king_bishop_rays{Bishop::attacks(cur_player[PieceType::King], total_occupied)},
      king_rook_rays{Rook::attacks(cur_player[PieceType::King], total_occupied)},
      pinners{compute_pinners()},
      pinned_pieces{compute_pinned_pieces()} {}

template <Color PlayerColor>
MoveContainer MoveGen<PlayerColor>::generate_moves() const {
  MoveContainer moves;
  Bitboard king_attackers = get_king_attackers();
  if (!king_attackers) {
    // King is not in check.
    piece::visit_non_king_pieces(
        [this, &moves]<PieceType PT>() { this->generate_unchecked_piece_moves<PT, MoveType::All>(moves); });
    generate_unchecked_king_moves<MoveType::All>(moves);
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
  piece::visit_non_king_pieces([this, &moves]<PieceType PT>() {
    this->generate_unchecked_piece_moves<PT, MoveType::CapturesAndPromotionsOnly>(moves);
  });
  generate_unchecked_king_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  return moves;
}

template <Color PlayerColor>
MoveContainer MoveGen<PlayerColor>::generate_quiescence_moves_and_checks() const {
  MoveContainer moves;
  piece::visit_non_king_pieces([this, &moves]<PieceType PT>() {
    this->generate_unchecked_piece_moves<PT, MoveType::CapturesChecksAndPromotionsOnly>(moves);
  });
  generate_unchecked_king_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);

  // The move generation above can only generate checks where the piece moved is the one giving check.
  // Another type of check is where the piece moves and another piece is the one giving check, which we generate below.

  // The piece at `from`, if moved to anywhere in `to_mask`, will cause a check by another piece.
  // Note that captures and promotions are ignored, as they are generated above already.
  auto generate_indirect_checks = [&](const Bitboard from, Bitboard to_mask) {
    const PieceType piece = cur_player.piece_at(from);
    if (pinned_pieces & from) to_mask &= cur_player[PieceType::King].ray(from);  // Constrain to pin ray.
    to_mask &= ~total_occupied;                                                  // Remove captures.
    piece::visit(piece, [this, from, &to_mask]<PieceType PT>() {
      to_mask &= get_piece_moves<PT>(from);  // Constrain to squares this piece can move to.
      to_mask &= ~get_opp_piece_attacks<PT>(opp_player[PieceType::King]);  // Remove checks.
    });
    if (piece == PieceType::Pawn) to_mask &= ~Pawn::get_promotion_squares<PlayerColor>();  // Remove promotions.

    for (const Bitboard to : to_mask.iterate()) {
      if (piece == PieceType::King && is_under_attack(to)) continue;  // Stop king from moving into check.
      moves.push_back(Move::move(from, to, piece, get_opp_piece_at(to)));
    }
  };

  const Bitboard opp_king = opp_player[PieceType::King];
  const Bitboard rook_ray_blockers{Rook::attacks(opp_king, total_occupied) & cur_occupied};
  const Bitboard rooks_attacking_opp_king = Rook::attacks(opp_king, total_occupied ^ rook_ray_blockers) &
                                            (cur_player[PieceType::Rook] | cur_player[PieceType::Queen]);
  for (const Bitboard rook : rooks_attacking_opp_king.iterate()) {
    const Bitboard between_rook_and_opp_king{opp_king.until(rook)};
    const Bitboard blocker{between_rook_and_opp_king & rook_ray_blockers};
    generate_indirect_checks(blocker, ~between_rook_and_opp_king);
  }

  const Bitboard bishop_ray_blockers{Bishop::attacks(opp_king, total_occupied) & cur_occupied};
  const Bitboard bishops_attacking_opp_king = Bishop::attacks(opp_king, total_occupied ^ bishop_ray_blockers) &
                                              (cur_player[PieceType::Bishop] | cur_player[PieceType::Queen]);
  for (const Bitboard bishop : bishops_attacking_opp_king.iterate()) {
    const Bitboard between_bishop_and_opp_king{opp_king.until(bishop)};
    const Bitboard blocker{between_bishop_and_opp_king & bishop_ray_blockers};
    generate_indirect_checks(blocker, ~between_bishop_and_opp_king);
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
    bool found_move{false};
    piece::visit_non_king_pieces([this, &found_move]<PieceType PT>() {
      if (found_move) return;
      found_move |= this->has_unchecked_piece_moves<PT>();
    });
    return found_move || has_unchecked_king_moves();
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
  const auto is_attacked_by = [this, square]<PieceType PT>() {
    return static_cast<bool>(get_piece_attacks<PT>(square) & opp_player[PT]);
  };
  bool is_attacked{false};
  piece::visit_all_pieces([&is_attacked, &is_attacked_by]<PieceType PT>() {
    if (is_attacked) return;
    is_attacked |= is_attacked_by.template operator()<PT>();
  });
  return is_attacked;
}

template <Color PlayerColor>
Bitboard MoveGen<PlayerColor>::compute_pinners() const {
  // NOTE: This function is called during initialization, not all members might be
  // initialized yet!
  const Bitboard current_slider_attackers =
      (king_bishop_rays & (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen])) |
      (king_rook_rays & (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]));
  const Bitboard potential_pinned_pieces = (king_bishop_rays | king_rook_rays) & cur_occupied;
  const Bitboard new_slider_attackers =
      (Bishop::attacks(cur_player[PieceType::King], total_occupied ^ potential_pinned_pieces) &
       (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen])) |
      (Rook::attacks(cur_player[PieceType::King], total_occupied ^ potential_pinned_pieces) &
       (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]));
  return new_slider_attackers ^ current_slider_attackers;
}

template <Color PlayerColor>
Bitboard MoveGen<PlayerColor>::compute_pinned_pieces() const {
  // NOTE: This function is called during initialization, not all members might be
  // initialized yet!
  Bitboard pinned_pieces{Bitboard::empty};
  for (const Bitboard pinner : pinners.iterate()) {
    const Bitboard pinned_piece = cur_player[PieceType::King].until(pinner) & cur_occupied;
    pinned_pieces ^= pinned_piece;
  }
  return pinned_pieces;
}

template <Color PlayerColor>
Bitboard MoveGen<PlayerColor>::get_king_attackers() const {
  Bitboard king = cur_player[PieceType::King];
  Bitboard attackers = (king_bishop_rays & (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen])) |
                       (Knight::attacks(king) & opp_player[PieceType::Knight]) |
                       (Pawn::attacks<PlayerColor>(king) & opp_player[PieceType::Pawn]) |
                       (king_rook_rays & (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]));
  return attackers;
}

template <Color PlayerColor>
template <PieceType PT>
Bitboard MoveGen<PlayerColor>::get_piece_moves(Bitboard square) const {
  if constexpr (Piece<PT>::is_slider()) {
    return Piece<PT>::attacks(square, total_occupied) & ~cur_occupied;
  } else if constexpr (PT == PieceType::Pawn) {
    const Bitboard captures{Pawn::attacks<PlayerColor>(square) & opp_occupied};
    const Bitboard moves{Pawn::pushes<PlayerColor>(square, total_occupied)};
    return captures | moves;
  } else {
    return Piece<PT>::attacks(square) & ~cur_occupied;
  }
}

template <Color PlayerColor>
template <PieceType PT>
Bitboard MoveGen<PlayerColor>::get_piece_attacks(Bitboard square) const {
  if constexpr (Piece<PT>::is_slider()) {
    return Piece<PT>::attacks(square, total_occupied);
  } else if constexpr (PT == PieceType::Pawn) {
    return Pawn::attacks<PlayerColor>(square);
  } else {
    return Piece<PT>::attacks(square);
  }
}

template <Color PlayerColor>
template <PieceType PT>
Bitboard MoveGen<PlayerColor>::get_opp_piece_attacks(Bitboard square) const {
  if constexpr (Piece<PT>::is_slider()) {
    return Piece<PT>::attacks(square, total_occupied);
  } else if constexpr (PT == PieceType::Pawn) {
    return Pawn::attacks<PlayerColor.flip()>(square);
  } else {
    return Piece<PT>::attacks(square);
  }
}

template <Color PlayerColor>
template <PieceType PT>
void MoveGen<PlayerColor>::add_move(MoveContainer& moves, Bitboard from, Bitboard to) const {
  const PieceType captured_piece{get_opp_piece_at(to)};
  if constexpr (PT == PieceType::Pawn) {
    if (to & Pawn::get_promotion_squares<PlayerColor>()) {
      moves.push_back(Move::promotion(from, to, PieceType::Bishop, captured_piece));
      moves.push_back(Move::promotion(from, to, PieceType::Knight, captured_piece));
      moves.push_back(Move::promotion(from, to, PieceType::Queen, captured_piece));
      moves.push_back(Move::promotion(from, to, PieceType::Rook, captured_piece));
      return;
    }
  }
  moves.push_back(Move::move(from, to, PT, captured_piece));
}

template <Color PlayerColor>
PieceType MoveGen<PlayerColor>::get_opp_piece_at(Bitboard bit) const {
  int index{bit.to_index()};
  int piece = (static_cast<uint64_t>(opp_piece_at[0] >> index) & 1) +
              ((static_cast<uint64_t>(opp_piece_at[1] >> index) & 1) << 1) +
              ((static_cast<uint64_t>(opp_piece_at[2] >> index) & 1) << 2);
  return static_cast<PieceType>(piece);
}

template <Color PlayerColor>
template <PieceType PT, typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_unchecked_piece_moves(MoveContainer& moves) const {
  // A piece can only move to certain squares to satisfy MoveType.
  const Bitboard to_mask = [=]() {
    if constexpr (MT == MoveType::All) return Bitboard::full;
    if constexpr (MT == MoveType::CapturesAndPromotionsOnly) {
      Bitboard mask{opp_occupied};
      if constexpr (PT == PieceType::Pawn) mask |= Pawn::get_promotion_squares<PlayerColor>();
      return mask;
    }
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      Bitboard mask{opp_occupied | get_opp_piece_attacks<PT>(opp_player[PieceType::King])};
      if constexpr (PT == PieceType::Pawn) mask |= Pawn::get_promotion_squares<PlayerColor>();
      return mask;
    }
  }();

  // Iterate through all pieces of the given type.
  for (const Bitboard from : cur_player[PT].iterate()) {
    Bitboard tos{get_piece_moves<PT>(from) & to_mask};
    if (from & pinned_pieces) tos &= cur_player[PieceType::King].ray(from);  // Pinned.
    for (const Bitboard to : tos.iterate()) add_move<PT>(moves, from, to);
  }

  // Check for en-passant.
  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would
  // be wrongly found to be legal.
  if constexpr (PT == PieceType::Pawn) {
    if (board.get_en_passant()) {
      const Bitboard froms{get_opp_piece_attacks<PieceType::Pawn>(board.get_en_passant()) &
                           cur_player[PieceType::Pawn]};
      for (const Bitboard from : froms.iterate()) {
        const Bitboard captured_pawn =
            (PlayerColor == Color::White) ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
        const Bitboard new_occupied{total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()};
        if (Bishop::attacks(cur_player[PieceType::King], new_occupied) &
            (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen]))
          continue;
        if (Rook::attacks(cur_player[PieceType::King], new_occupied) &
            (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]))
          continue;
        moves.push_back(Move::move(from, board.get_en_passant(), PieceType::Pawn, PieceType::Pawn));
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

  const Bitboard king = cur_player[PieceType::King];
  Bitboard rook_to_mask = Bitboard::full;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    rook_to_mask = Rook::attacks(opp_player[PieceType::King], total_occupied);
  }
  if (cur_player.can_castle_kingside() && !(total_occupied & (king << 1 | king << 2)) && !is_under_attack(king << 1) &&
      !is_under_attack(king << 2) && (rook_to_mask & king << 1)) {
    moves.push_back(Move::move(king, king << 2, PieceType::King));
  }
  if (cur_player.can_castle_queenside() && !(total_occupied & (king >> 1 | king >> 2 | king >> 3)) &&
      !is_under_attack(king >> 1) && !is_under_attack(king >> 2) && (rook_to_mask & king >> 1)) {
    moves.push_back(Move::move(king, king >> 2, PieceType::King));
  }
}

template <Color PlayerColor>
void MoveGen<PlayerColor>::generate_king_single_check_evasions(MoveContainer& moves, Bitboard attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  // 1. Capture the attacker with a piece that is not pinned.
  const auto capture_attacker_with_piece{[this, attacker, &moves]<PieceType PT>() {
    const Bitboard capturers{get_opp_piece_attacks<PT>(attacker) & cur_player[PT] & ~pinned_pieces};
    for (const Bitboard capturer : capturers.iterate()) {
      add_move<PT>(moves, capturer, attacker);
    }
  }};
  piece::visit_non_king_pieces(capture_attacker_with_piece);

  // Special en-passant check for pawns.
  if (attacker == (PlayerColor == Color::White ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    Bitboard capturing_square = PlayerColor == Color::White ? attacker << 8 : attacker >> 8;
    Bitboard pawn_capturers =
        get_opp_piece_attacks<PieceType::Pawn>(capturing_square) & cur_player[PieceType::Pawn] & ~pinned_pieces;
    for (const Bitboard from : pawn_capturers.iterate()) {
      moves.push_back(Move::move(from, capturing_square, PieceType::Pawn, PieceType::Pawn));
    }
  }

  // 2. King moves to a square that is not attacked. This is the same as evading
  // double check.
  generate_king_double_check_evasions<MoveType::All>(moves);

  // 3. Block the check if it is a sliding check, with a piece that is not pinned.
  if (piece::is_slider(get_opp_piece_at(attacker))) {
    Bitboard blocking_squares{cur_player[PieceType::King].until(attacker)};
    const auto block_with_piece{[this, blocking_squares, &moves]<PieceType PT>() {
      const Bitboard froms{cur_player[PT] & ~pinned_pieces};
      for (const Bitboard from : froms.iterate()) {
        const Bitboard tos{get_piece_moves<PT>(from) & blocking_squares};
        for (const Bitboard to : tos.iterate()) {
          add_move<PT>(moves, from, to);
        }
      }
    }};
    piece::visit_non_king_pieces(block_with_piece);
  }
}

template <Color PlayerColor>
template <typename MoveGen<PlayerColor>::MoveType MT>
void MoveGen<PlayerColor>::generate_king_double_check_evasions(MoveContainer& moves) const {
  // To evade a double check, the king must move to a square that is not attacked.
  const Bitboard king = cur_player[PieceType::King];
  Bitboard to_bitboard = King::attacks(king) & ~cur_occupied &
                         ~Pawn::attacks<PlayerColor.flip()>(opp_player[PieceType::Pawn]) &
                         ~King::attacks(opp_player[PieceType::King]);
  if constexpr (MT != MoveType::All) to_bitboard &= opp_occupied;
  const Bitboard total_occupied_without_king = total_occupied ^ king;
  for (const Bitboard to : to_bitboard.iterate()) {
    if (Knight::attacks(to) & opp_player[PieceType::Knight]) continue;
    if (Bishop::attacks(to, total_occupied_without_king) &
        (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen]))
      continue;
    if (Rook::attacks(to, total_occupied_without_king) & (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]))
      continue;
    moves.push_back(Move::move(king, to, PieceType::King, get_opp_piece_at(to)));
  }
}

template <Color PlayerColor>
template <PieceType PT>
bool MoveGen<PlayerColor>::has_unchecked_piece_moves() const {
  // Iterate through all pieces of the given type.
  for (const Bitboard from : cur_player[PT].iterate()) {
    Bitboard tos{get_piece_moves<PT>(from)};
    if (from & pinned_pieces) tos &= cur_player[PieceType::King].ray(from);  // Pinned.
    if (tos) return true;
  }

  // Check for en-passant.
  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would
  // be wrongly found to be legal.
  if constexpr (PT == PieceType::Pawn) {
    if (board.get_en_passant()) {
      const Bitboard froms{get_opp_piece_attacks<PieceType::Pawn>(board.get_en_passant()) &
                           cur_player[PieceType::Pawn]};
      for (const Bitboard from : froms.iterate()) {
        const Bitboard captured_pawn =
            (PlayerColor == Color::White) ? board.get_en_passant() >> 8 : board.get_en_passant() << 8;
        const Bitboard new_occupied{total_occupied ^ from ^ captured_pawn ^ board.get_en_passant()};
        if (Bishop::attacks(cur_player[PieceType::King], new_occupied) &
            (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen]))
          continue;
        if (Rook::attacks(cur_player[PieceType::King], new_occupied) &
            (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]))
          continue;
        return true;
      }
    }
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_unchecked_king_moves() const {
  // The king is not in check, hence it either moves to an unattacked square, or
  // we may castle.
  if (has_king_double_check_evasions()) return true;

  const Bitboard king = cur_player[PieceType::King];
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
bool MoveGen<PlayerColor>::has_king_single_check_evasions(Bitboard attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  // 1. Capture the attacker with a piece that is not pinned.
  bool found_capture{false};
  const auto capture_attacker_with_piece{[this, attacker, &found_capture]<PieceType PT>() {
    if (found_capture) return;
    const Bitboard capturers{get_opp_piece_attacks<PT>(attacker) & cur_player[PT] & ~pinned_pieces};
    if (capturers) found_capture = true;
  }};
  piece::visit_non_king_pieces(capture_attacker_with_piece);
  if (found_capture) return true;

  // Special en-passant check for pawns.
  if (attacker == (PlayerColor == Color::White ? board.get_en_passant() >> 8 : board.get_en_passant() << 8)) {
    Bitboard capturing_square = PlayerColor == Color::White ? attacker << 8 : attacker >> 8;
    Bitboard pawn_capturers =
        get_opp_piece_attacks<PieceType::Pawn>(capturing_square) & cur_player[PieceType::Pawn] & ~pinned_pieces;
    if (pawn_capturers) return true;
  }

  // 2. King moves to a square that is not attacked. This is the same as evading
  // double check.
  if (has_king_double_check_evasions()) return true;

  // 3. Block the check if it is a sliding check, with a piece that is not pinned.
  if (piece::is_slider(get_opp_piece_at(attacker))) {
    bool found_blocker{false};
    const Bitboard blocking_squares{cur_player[PieceType::King].until(attacker)};
    const auto block_with_piece{[this, blocking_squares, &found_blocker]<PieceType PT>() {
      if (found_blocker) return;
      const Bitboard froms{cur_player[PT] & ~pinned_pieces};
      for (const Bitboard from : froms.iterate()) {
        const Bitboard tos{get_piece_moves<PT>(from) & blocking_squares};
        if (tos) found_blocker = true;
      }
    }};
    piece::visit_non_king_pieces(block_with_piece);
    if (found_blocker) return true;
  }

  return false;
}

template <Color PlayerColor>
bool MoveGen<PlayerColor>::has_king_double_check_evasions() const {
  // To evade a double check, the king must move to a square that is not attacked.
  const Bitboard king = cur_player[PieceType::King];
  const Bitboard to_bitboard = King::attacks(king) & ~cur_occupied &
                               ~Pawn::attacks<PlayerColor.flip()>(opp_player[PieceType::Pawn]) &
                               ~King::attacks(opp_player[PieceType::King]);
  const Bitboard total_occupied_without_king = total_occupied ^ king;
  for (const Bitboard to : to_bitboard.iterate()) {
    if (Knight::attacks(to) & opp_player[PieceType::Knight]) continue;
    if (Bishop::attacks(to, total_occupied_without_king) &
        (opp_player[PieceType::Bishop] | opp_player[PieceType::Queen]))
      continue;
    if (Rook::attacks(to, total_occupied_without_king) & (opp_player[PieceType::Rook] | opp_player[PieceType::Queen]))
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
