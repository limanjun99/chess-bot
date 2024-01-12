#include "board.h"

#include <cctype>

Board::Board(Player white, Player black, u64 en_passant_bit, bool is_white_turn)
    : white{white},
      black{black},
      en_passant_bit{en_passant_bit},
      cur_occupied{is_white_turn ? white.occupied() : black.occupied()},
      opp_occupied{is_white_turn ? black.occupied() : white.occupied()},
      is_white_turn{is_white_turn} {}

Board Board::initial() { return Board(Player::white_initial(), Player::black_initial(), 0, true); }

Board Board::from_epd(std::string_view epd) {
  Player white{0, 0, 0, 0, 0, 0, false, false};
  Player black{0, 0, 0, 0, 0, 0, false, false};
  size_t index = 0;

  // Pieces.
  for (int y = 7; y >= 0; y--) {
    int x = 0;
    while (epd[index] != '/' && epd[index] != ' ') {
      char ch = epd[index];
      index++;
      if (std::isdigit(ch)) {
        x += static_cast<int>(ch - '0');
        continue;
      }
      bool is_white_piece = std::isupper(ch);
      Piece piece = piece::from_char(ch);
      Player& player = is_white_piece ? white : black;
      player[piece] |= u64(1) << (y * 8 + x);
      x++;
    }
    index++;
  }

  // Side to move.
  bool is_white_turn = epd[index] == 'w';
  index += 2;

  // Castling rights.
  while (epd[index] != '-' && epd[index] != ' ') {
    bool is_white = std::isupper(epd[index]);
    Player& player = is_white ? white : black;
    if (epd[index] == 'K' || epd[index] == 'k')
      player.enable_kingside_castling();
    else
      player.enable_queenside_castling();
    index++;
  }
  if (epd[index] == '-') index++;
  index++;

  // En passant target square.
  u64 en_passant_bit = 0;
  if (epd[index] != '-') en_passant_bit = bit::from_algebraic(epd.substr(index, 2));

  return Board{white, black, en_passant_bit, is_white_turn};
}

Board Board::apply_move(const Move& move) const {
  if (move.is_promotion()) return apply_promotion(move.get_from(), move.get_to(), move.get_promotion_piece());
  return apply_move(move.get_piece(), move.get_from(), move.get_to());
}

Board Board::apply_move(Piece piece, u64 from, u64 to) const {
  Board board = *this;
  Player& cur = board.cur_player();
  Player& opp = board.opp_player();
  cur[piece] ^= from | to;
  board.cur_occupied ^= from | to;

  // Update castling flag.
  if (opp[Piece::Rook] & to) {
    if (to == (is_white_turn ? bitboard::H8 : bitboard::H1)) opp.disable_kingside_castling();
    if (to == (is_white_turn ? bitboard::A8 : bitboard::A1)) opp.disable_queenside_castling();
  }
  if (piece == Piece::King) {
    cur.disable_castling();
  } else if (piece == Piece::Rook) {
    if (from == (is_white_turn ? bitboard::H1 : bitboard::H8)) cur.disable_kingside_castling();
    if (from == (is_white_turn ? bitboard::A1 : bitboard::A8)) cur.disable_queenside_castling();
  }

  opp &= ~to;
  board.opp_occupied &= ~to;

  // Handle castling.
  if (piece == Piece::King) {
    if (to == from << 2) {  // Kingside castling.
      cur[Piece::Rook] ^= from << 1 | from << 3;
      board.cur_occupied ^= from << 1 | from << 3;
    } else if (to == from >> 2) {  // Queenside castling.
      cur[Piece::Rook] ^= from >> 1 | from >> 4;
      board.cur_occupied ^= from >> 1 | from >> 4;
    }
  }

  // Handle en passant's capture.
  if (piece == Piece::Pawn && to == en_passant_bit) {
    opp[Piece::Pawn] ^= is_white_turn ? en_passant_bit >> 8 : en_passant_bit << 8;
    board.opp_occupied ^= is_white_turn ? en_passant_bit >> 8 : en_passant_bit << 8;
  }

  // Update en passant flag.
  board.en_passant_bit = 0;
  if (piece == Piece::Pawn) {
    if (to == from << 16) {
      board.en_passant_bit = from << 8;
    } else if (to == from >> 16) {
      board.en_passant_bit = from >> 8;
    }
  }

  board.is_white_turn = !board.is_white_turn;
  std::swap(board.cur_occupied, board.opp_occupied);

  return board;
}

Board Board::apply_promotion(u64 from, u64 to, Piece piece) const {
  Board board = apply_move(Piece::Pawn, from, to);
  board.opp_player()[piece] ^= to;
  board.opp_player()[Piece::Pawn] ^= to;
  return board;
}

Board Board::apply_uci_move(std::string_view uci_move) {
  u64 from = bit::from_algebraic(uci_move.substr(0, 2));
  u64 to = bit::from_algebraic(uci_move.substr(2, 2));
  if (uci_move.size() == 5) {
    Piece promotion_piece = piece::from_char(uci_move[4]);
    return apply_promotion(from, to, promotion_piece);
  } else {
    Piece piece = cur_player().piece_at(from);
    return apply_move(piece, from, to);
  }
}

const Player& Board::cur_player() const { return is_white_turn ? white : black; }

Player& Board::cur_player() { return is_white_turn ? white : black; }

std::vector<Move> Board::generate_quiescence_moves() const {
  std::vector<Move> moves;
  moves.reserve(64);
  const u64 pinned_pieces = get_pinned_pieces();
  generate_unchecked_bishoplike_moves<MoveType::CapturesAndPromotionsOnly>(moves, pinned_pieces);
  generate_unchecked_king_moves<MoveType::CapturesAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesAndPromotionsOnly>(moves, pinned_pieces);
  generate_unchecked_pawn_moves<MoveType::CapturesAndPromotionsOnly>(moves, pinned_pieces);
  generate_unchecked_rooklike_moves<MoveType::CapturesAndPromotionsOnly>(moves, pinned_pieces);
  return moves;
}

std::vector<Move> Board::generate_quiescence_moves_and_checks() const {
  std::vector<Move> moves;
  moves.reserve(64);
  const u64 pinned_pieces = get_pinned_pieces();
  generate_unchecked_bishoplike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves, pinned_pieces);
  generate_unchecked_king_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves);
  generate_unchecked_knight_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves, pinned_pieces);
  generate_unchecked_pawn_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves, pinned_pieces);
  generate_unchecked_rooklike_moves<MoveType::CapturesChecksAndPromotionsOnly>(moves, pinned_pieces);

  // The move generation above can only generate checks where the piece moved is the one giving check.
  // Another type of check is where the piece moves and another piece is the one giving check, which we generate below.
  const u64 total_occupied = cur_occupied | opp_occupied;
  const u64 opp_king_bishop_rays = bitboard::bishop_attacks(opp_player()[Piece::King], total_occupied);
  const u64 opp_king_rook_rays = bitboard::rook_attacks(opp_player()[Piece::King], total_occupied);
  const u64 cur_king_attackers = get_king_attackers();

  // The piece at `from`, if moved to anywhere in `to_mask`, will cause a check by another piece.
  // Note that captures and promotions are ignored, as they are generated above already.
  auto generate_indirect_checks = [&](const u64 from, u64 to_mask) {
    const Piece piece = cur_player().piece_at(from);
    if (pinned_pieces & from) {
      // The piece is pinned to its own king, can only move along the pin ray.
      const u64 new_king_attackers = bitboard::queen_attacks(cur_player()[Piece::King], total_occupied ^ from);
      const u64 pinner = new_king_attackers ^ cur_king_attackers;
      to_mask &= bitboard::block_slider_check(cur_player()[Piece::King], pinner);
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
        to_mask &= ~bitboard::knight_attacks(opp_player()[Piece::King]) & bitboard::knight_attacks(from);
        break;
      case Piece::Pawn:
        to_mask &= ~bitboard::pawn_attacks(opp_player()[Piece::King], !is_white_turn) & ~bitboard::RANK_1 &
                   ~bitboard::RANK_8 & bitboard::pawn_pushes(from, total_occupied, is_white_turn);
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
      bitboard::iterate(to_mask, [&](const u64 to) {
        if (bitboard::bishop_attacks(to, total_occupied) & (opp_player()[Piece::Bishop] | opp_player()[Piece::Queen]))
          return;
        if (bitboard::king_attacks(to) & opp_player()[Piece::King]) return;
        if (bitboard::knight_attacks(to) & opp_player()[Piece::Knight]) return;
        if (bitboard::pawn_attacks(to, is_white_turn) & opp_player()[Piece::Pawn]) return;
        if (bitboard::rook_attacks(to, total_occupied) & (opp_player()[Piece::Rook] | opp_player()[Piece::Queen]))
          return;
        moves.emplace_back(piece, from, to);
      });
    } else {
      bitboard::iterate(to_mask, [&](const u64 to) { moves.emplace_back(piece, from, to); });
    }
  };

  const u64 maybe_self_pinned_by_rooks = opp_king_rook_rays & cur_occupied;
  const u64 rook_attackers = opp_king_rook_rays & (cur_player()[Piece::Rook] | cur_player()[Piece::Queen]);
  bitboard::iterate(maybe_self_pinned_by_rooks, [&](const u64 from) {
    const u64 new_rook_rays = bitboard::rook_attacks(opp_player()[Piece::King], total_occupied ^ from);
    const u64 new_rook_attackers = new_rook_rays & (cur_player()[Piece::Rook] | cur_player()[Piece::Queen]);
    if (new_rook_attackers == rook_attackers) return;
    u64 to_mask = ~bitboard::block_slider_check(opp_player()[Piece::King], new_rook_attackers ^ rook_attackers);
    generate_indirect_checks(from, to_mask);
  });
  const u64 maybe_self_pinned_by_bishops = opp_king_bishop_rays & cur_occupied;
  const u64 bishop_attackers = opp_king_bishop_rays & (cur_player()[Piece::Bishop] | cur_player()[Piece::Queen]);
  bitboard::iterate(maybe_self_pinned_by_bishops, [&](const u64 from) {
    const u64 new_bishop_rays = bitboard::bishop_attacks(opp_player()[Piece::King], total_occupied ^ from);
    const u64 new_bishop_attackers = new_bishop_rays & (cur_player()[Piece::Bishop] | cur_player()[Piece::Queen]);
    if (new_bishop_attackers == bishop_attackers) return;
    u64 to_mask = ~bitboard::block_slider_check(opp_player()[Piece::King], new_bishop_attackers ^ bishop_attackers);
    generate_indirect_checks(from, to_mask);
  });

  return moves;
}

std::vector<Move> Board::generate_moves() const {
  std::vector<Move> moves;
  moves.reserve(64);
  // Occupancy is precomputed to avoid having to repeat it in each piece's move generation function.
  u64 king_attackers = get_king_attackers();
  if (king_attackers == 0) {
    // King is not in check.
    // Note that queen moves are generated in both generate_unchecked_bishoplike_moves() and
    // generate_unchecked_rooklike_moves().
    u64 pinned_pieces = get_pinned_pieces();
    generate_unchecked_bishoplike_moves<MoveType::All>(moves, pinned_pieces);
    generate_unchecked_king_moves<MoveType::All>(moves);
    generate_unchecked_knight_moves<MoveType::All>(moves, pinned_pieces);
    generate_unchecked_pawn_moves<MoveType::All>(moves, pinned_pieces);
    generate_unchecked_rooklike_moves<MoveType::All>(moves, pinned_pieces);
  } else if (bitboard::count(king_attackers) == 1) {
    // King is in single-check.
    generate_king_single_check_evasions(moves, king_attackers);
  } else {
    // King is in double-check.
    generate_king_double_check_evasions<MoveType::All>(moves);
  }
  return moves;
}

MoveSet Board::generate_moveset() const {
  MoveSet move_set{*this};
  generate_bishop_moves(move_set);
  generate_king_moves(move_set);
  generate_knight_moves(move_set);
  generate_pawn_moves(move_set);
  generate_queen_moves(move_set);
  generate_rook_moves(move_set);
  return move_set;
}

bool Board::is_a_capture(const Move& move) const {
  return (move.get_to() & opp_occupied) || (move.get_piece() == Piece::Pawn && move.get_to() == en_passant_bit);
}

bool Board::is_a_check(const Move& move) const {
  // This function is not optimized as it should only be called in non-critical code for external interaction.
  return apply_move(move).is_in_check();
}

bool Board::is_in_check() const { return is_under_attack(cur_player()[Piece::King]); }

bool Board::is_under_attack(u64 square) const {
  const Player& opp = opp_player();
  u64 total_occupied = cur_occupied | opp_occupied;
  if (bitboard::bishop_attacks(square, total_occupied) & (opp[Piece::Bishop] | opp[Piece::Queen])) {
    return true;
  }
  if (bitboard::king_attacks(square) & opp[Piece::King]) return true;
  if (bitboard::knight_attacks(square) & opp[Piece::Knight]) return true;
  if (bitboard::pawn_attacks(square, is_white_turn) & opp[Piece::Pawn]) return true;
  if (bitboard::rook_attacks(square, total_occupied) & (opp[Piece::Rook] | opp[Piece::Queen])) {
    return true;
  }
  return false;
}

bool Board::is_white_to_move() const { return is_white_turn; }

bool Board::moved_into_check() const {
  const Player& cur = cur_player();
  const Player& opp = opp_player();
  u64 king = opp[Piece::King];
  u64 total_occupied = cur_occupied | opp_occupied;
  if (bitboard::bishop_attacks(king, total_occupied) & (cur[Piece::Bishop] | cur[Piece::Queen])) {
    return true;
  }
  if (bitboard::king_attacks(king) & cur[Piece::King]) return true;
  if (bitboard::knight_attacks(king) & cur[Piece::Knight]) return true;
  if (bitboard::pawn_attacks(king, !is_white_turn) & cur[Piece::Pawn]) return true;
  if (bitboard::rook_attacks(king, total_occupied) & (cur[Piece::Rook] | cur[Piece::Queen])) {
    return true;
  }
  return false;
}

const Player& Board::opp_player() const { return is_white_turn ? black : white; }

Player& Board::opp_player() { return is_white_turn ? black : white; }

std::string Board::to_string() const {
  std::string s;
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      u64 bit = u64(1) << (y * 8 + x);
      constexpr char piece_char[6] = {'b', 'k', 'n', 'p', 'q', 'r'};
      if (white.occupied() & bit) {
        s += piece_char[static_cast<int>(white.piece_at(bit))] - 32;
      } else if (black.occupied() & bit) {
        s += piece_char[static_cast<int>(black.piece_at(bit))];
      } else {
        s += '.';
      }
    }
    if (y) s += '\n';
  }
  return s;
}

void Board::add_pawn_moves(std::vector<Move>& moves, u64 from, u64 to) const {
  if (to & (bitboard::RANK_1 | bitboard::RANK_8)) {
    moves.emplace_back(from, to, Piece::Bishop);
    moves.emplace_back(from, to, Piece::Knight);
    moves.emplace_back(from, to, Piece::Queen);
    moves.emplace_back(from, to, Piece::Rook);
  } else {
    moves.emplace_back(Piece::Pawn, from, to);
  }
}

u64 Board::get_king_attackers() const {
  u64 king = cur_player()[Piece::King];
  u64 total_occupied = cur_occupied | opp_occupied;
  const Player& opp = opp_player();

  u64 attackers = (bitboard::bishop_attacks(king, total_occupied) & (opp[Piece::Bishop] | opp[Piece::Queen])) |
                  (bitboard::knight_attacks(king) & opp[Piece::Knight]) |
                  (bitboard::pawn_attacks(king, is_white_turn) & opp[Piece::Pawn]) |
                  (bitboard::rook_attacks(king, total_occupied) & (opp[Piece::Rook] | opp[Piece::Queen]));
  return attackers;
}

u64 Board::get_pinned_pieces() const {
  const u64 king = cur_player()[Piece::King];
  const u64 total_occupied = cur_occupied | opp_occupied;
  const u64 bishop_attacks = bitboard::bishop_attacks(king, total_occupied);
  const u64 rook_attacks = bitboard::rook_attacks(king, total_occupied);
  const u64 bishop_attackers = bishop_attacks & (opp_player()[Piece::Bishop] | opp_player()[Piece::Queen]);
  const u64 rook_attackers = rook_attacks & (opp_player()[Piece::Rook] | opp_player()[Piece::Queen]);
  u64 pinned_pieces = 0;

  const u64 potential_bishop_pins = bishop_attacks & cur_occupied;
  bitboard::iterate(potential_bishop_pins, [this, bishop_attackers, king, &pinned_pieces, total_occupied](u64 pinned) {
    // Check if removing the piece opens up the king to a new bishop / queen attacker.
    const u64 new_bishop_attackers = bitboard::bishop_attacks(king, total_occupied ^ pinned) &
                                     (opp_player()[Piece::Bishop] | opp_player()[Piece::Queen]);
    if (new_bishop_attackers == bishop_attackers) return;
    pinned_pieces ^= pinned;
  });

  const u64 potential_rook_pins = rook_attacks & cur_occupied;
  bitboard::iterate(potential_rook_pins, [this, rook_attackers, king, &pinned_pieces, total_occupied](u64 pinned) {
    // Check if removing the piece opens up the king to a new rook / queen attacker.
    const u64 new_rook_attackers = bitboard::rook_attacks(king, total_occupied ^ pinned) &
                                   (opp_player()[Piece::Rook] | opp_player()[Piece::Queen]);
    if (new_rook_attackers == rook_attackers) return;
    pinned_pieces ^= pinned;
  });

  return pinned_pieces;
}

void Board::generate_bishop_moves(MoveSet& move_set) const {
  u64 bishops = cur_player()[Piece::Bishop];
  u64 total_occupied = cur_occupied | opp_occupied;
  while (bishops) {
    u64 bishop = bit::lsb(bishops);
    bishops ^= bishop;
    u64 to_bitmap = bitboard::bishop_attacks(bishop, total_occupied) & ~cur_occupied;
    move_set.add_piece_moves(Piece::Bishop, bishop, to_bitmap);
  }
}

template <Board::MoveType MT>
void Board::generate_unchecked_bishoplike_moves(std::vector<Move>& moves, u64 pinned_pieces) const {
  // Bishops that are pinned along a rook ray cannot move at all.
  // Bishops that are pinned along a bishop ray can only move along that ray.
  // Un-pinned bishops can move anywhere.
  const u64 total_occupied = cur_occupied | opp_occupied;
  const u64 king_bishop_rays = bitboard::bishop_attacks(cur_player()[Piece::King], total_occupied);
  const u64 opp_blockers = king_bishop_rays & opp_occupied;  // Opponent pieces that block bishop rays from the king.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::bishop_attacks(opp_player()[Piece::King], total_occupied);
  }

  for (Piece from_piece : {Piece::Bishop, Piece::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == Piece::Queen) to_mask |= bitboard::queen_attacks(opp_player()[Piece::King], total_occupied);
    }
    const u64 unpinned_pieces = cur_player()[from_piece] & ~pinned_pieces;
    bitboard::iterate(unpinned_pieces, [&](const u64 from) {
      const u64 to_bitboard = bitboard::bishop_attacks(from, total_occupied) & ~cur_occupied & to_mask;
      bitboard::iterate(to_bitboard, [&](const u64 to) { moves.emplace_back(from_piece, from, to); });
    });

    const u64 pinned_from_pieces = king_bishop_rays & pinned_pieces & cur_player()[from_piece];
    bitboard::iterate(pinned_from_pieces, [&](const u64 from) {
      const u64 pinned_by = bitboard::bishop_attacks(cur_player()[Piece::King], total_occupied ^ from) &
                            (opp_player()[Piece::Bishop] | opp_player()[Piece::Queen]) & ~opp_blockers;
      if (pinned_by == 0) return;
      const u64 to_bitboard =
          (bitboard::block_slider_check(cur_player()[Piece::King], pinned_by) | pinned_by) & ~from & to_mask;
      bitboard::iterate(to_bitboard, [&](const u64 to) { moves.emplace_back(from_piece, from, to); });
    });
  }
}

void Board::generate_king_moves(MoveSet& move_set) const {
  u64 king = cur_player()[Piece::King];
  u64 total_occupied = cur_occupied | opp_occupied;
  u64 to_bitmap = bitboard::king_attacks(king) & ~cur_occupied;
  if (cur_player().can_castle_kingside() && !(total_occupied & (king << 1 | king << 2)) && !is_under_attack(king) &&
      !is_under_attack(king << 1)) {
    to_bitmap |= king << 2;
  }
  if (cur_player().can_castle_queenside() && !(total_occupied & (king >> 1 | king >> 2 | king >> 3)) &&
      !is_under_attack(king) && !is_under_attack(king >> 1)) {
    to_bitmap |= king >> 2;
  }
  move_set.add_piece_moves(Piece::King, king, to_bitmap);
}

template <Board::MoveType MT>
void Board::generate_unchecked_king_moves(std::vector<Move>& moves) const {
  // The king is not in check, hence it either moves to an unattacked square, or we may castle.
  generate_king_double_check_evasions<MT>(moves);

  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) return;

  const u64 king = cur_player()[Piece::King];
  const u64 total_occupied = cur_occupied | opp_occupied;
  u64 rook_to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    rook_to_mask = bitboard::rook_attacks(opp_player()[Piece::King], total_occupied);
  }
  if (cur_player().can_castle_kingside() && ((total_occupied & (king << 1 | king << 2)) == 0) &&
      !is_under_attack(king << 1) && !is_under_attack(king << 2) && (rook_to_mask & king << 1)) {
    moves.emplace_back(Piece::King, king, king << 2);
  }
  if (cur_player().can_castle_queenside() && ((total_occupied & (king >> 1 | king >> 2 | king >> 3)) == 0) &&
      !is_under_attack(king >> 1) && !is_under_attack(king >> 2) && (rook_to_mask & king >> 1)) {
    moves.emplace_back(Piece::King, king, king >> 2);
  }
}

void Board::generate_knight_moves(MoveSet& move_set) const {
  u64 knights = cur_player()[Piece::Knight];
  while (knights) {
    u64 knight = bit::lsb(knights);
    knights ^= knight;
    u64 to_bitmap = bitboard::knight_attacks(knight) & ~cur_occupied;
    move_set.add_piece_moves(Piece::Knight, knight, to_bitmap);
  }
}

template <Board::MoveType MT>
void Board::generate_unchecked_knight_moves(std::vector<Move>& moves, u64 pinned_pieces) const {
  // Pinned knights cannot move, while un-pinned knights can move anywhere.
  const u64 knights = cur_player()[Piece::Knight] & ~pinned_pieces;
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::knight_attacks(opp_player()[Piece::King]);
  }
  bitboard::iterate(knights, [&](const u64 from) {
    u64 to_bitboard = bitboard::knight_attacks(from) & ~cur_occupied & to_mask;
    bitboard::iterate(to_bitboard, [from, &moves](const u64 to) { moves.emplace_back(Piece::Knight, from, to); });
  });
}

void Board::generate_pawn_moves(MoveSet& move_set) const {
  const Player& cur = cur_player();
  u64 pawns = cur[Piece::Pawn];
  u64 opp_capturable = opp_occupied | en_passant_bit;
  u64 total_occupied = cur_occupied | opp_occupied;
  while (pawns) {
    u64 pawn = bit::lsb(pawns);
    pawns ^= pawn;
    u64 to_bitmap = bitboard::pawn_attacks(pawn, is_white_turn) & opp_capturable;  // Captures.
    u64 push = is_white_turn ? pawn << 8 : pawn >> 8;
    u64 double_push = is_white_turn ? (pawn & bitboard::RANK_2) << 16 : (pawn & bitboard::RANK_7) >> 16;
    to_bitmap |= push & ~total_occupied;
    to_bitmap |= (push & ~total_occupied) ? (double_push & ~total_occupied) : 0;
    move_set.add_piece_moves(Piece::Pawn, pawn, to_bitmap);
  }
}

template <Board::MoveType MT>
void Board::generate_unchecked_pawn_moves(std::vector<Move>& moves, u64 pinned_pieces) const {
  // Un-pinned pawns can move anywhere.
  // Pawns that are pinned by a rook ray can only push forward.
  // Pawns that are pinned by a bishop ray can only capture in the direction of the pinner.
  const u64 total_occupied = cur_occupied | opp_occupied;
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) {
    to_mask = opp_occupied | (is_white_turn ? bitboard::RANK_8 : bitboard::RANK_1);
  }
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = bitboard::pawn_attacks(opp_player()[Piece::King], !is_white_turn) | opp_occupied |
              (is_white_turn ? bitboard::RANK_8 : bitboard::RANK_1);
  }

  const u64 unpinned_pawns = cur_player()[Piece::Pawn] & ~pinned_pieces;
  bitboard::iterate(unpinned_pawns, [&](const u64 from) {
    u64 to_bitboard = bitboard::pawn_attacks(from, is_white_turn) & opp_occupied;
    if (is_white_turn) {
      to_bitboard |=
          (from << 8 & ~total_occupied) | ((from & bitboard::RANK_2) << 16 & ~total_occupied & ~(total_occupied << 8));
    } else {
      to_bitboard |=
          (from >> 8 & ~total_occupied) | ((from & bitboard::RANK_7) >> 16 & ~total_occupied & ~(total_occupied >> 8));
    }
    to_bitboard &= to_mask;
    bitboard::iterate(to_bitboard, [this, from, &moves](const u64 to) { add_pawn_moves(moves, from, to); });
  });

  const u64 king_bishop_rays = bitboard::bishop_attacks(cur_player()[Piece::King], total_occupied);
  const u64 king_rook_rays = bitboard::rook_attacks(cur_player()[Piece::King], total_occupied);
  const u64 bishop_ray_blockers = king_bishop_rays & opp_occupied;
  const u64 rook_ray_blockers = king_rook_rays & opp_occupied;
  const u64 pinned_pawns = (king_bishop_rays | king_rook_rays) & pinned_pieces & cur_player()[Piece::Pawn];
  bitboard::iterate(pinned_pawns, [&](const u64 from) {
    if (from & king_bishop_rays) {
      // Pinned along a bishop ray.
      const u64 pinned_by = bitboard::bishop_attacks(cur_player()[Piece::King], total_occupied ^ from) &
                            (opp_player()[Piece::Bishop] | opp_player()[Piece::Queen]) & ~bishop_ray_blockers;
      if (pinned_by == 0) return;
      if ((pinned_by & bitboard::pawn_attacks(from, is_white_turn)) == 0) return;
      add_pawn_moves(moves, from, pinned_by);
    } else {
      // Pinned along a rook ray.
      u64 to_bitboard = bitboard::pawn_pushes(from, total_occupied, is_white_turn) & to_mask;
      const u64 rook = bitboard::rook_attacks(cur_player()[Piece::King], total_occupied ^ from) &
                       (opp_player()[Piece::Rook] | opp_player()[Piece::Queen]) & ~rook_ray_blockers;
      to_bitboard &= bitboard::block_slider_check(cur_player()[Piece::King], rook);
      bitboard::iterate(to_bitboard, [this, from, &moves](const u64 to) { add_pawn_moves(moves, from, to); });
    }
  });

  // Note that en-passant cannot be validated by pinned pieces.
  // For example, the case "K..pP..r", where "p" can be captured en-passant, would be wrongly found to be legal.
  if (en_passant_bit) {
    const u64 from_bitboard = bitboard::pawn_attacks(en_passant_bit, !is_white_turn) & cur_player()[Piece::Pawn];
    bitboard::iterate(from_bitboard, [this, &moves, total_occupied](const u64 from) {
      const u64 captured_pawn = is_white_turn ? en_passant_bit >> 8 : en_passant_bit << 8;
      if (bitboard::bishop_attacks(cur_player()[Piece::King], total_occupied ^ from ^ captured_pawn ^ en_passant_bit) &
          (opp_player()[Piece::Bishop] | opp_player()[Piece::Queen]))
        return;
      if (bitboard::rook_attacks(cur_player()[Piece::King], total_occupied ^ from ^ captured_pawn ^ en_passant_bit) &
          (opp_player()[Piece::Rook] | opp_player()[Piece::Queen]))
        return;
      add_pawn_moves(moves, from, en_passant_bit);
    });
  }
}

void Board::generate_queen_moves(MoveSet& move_set) const {
  u64 queens = cur_player()[Piece::Queen];
  u64 total_occupied = cur_occupied | opp_occupied;
  while (queens) {
    u64 queen = bit::lsb(queens);
    queens ^= queen;
    u64 to_bitmap = (bitboard::bishop_attacks(queen, total_occupied) | bitboard::rook_attacks(queen, total_occupied)) &
                    ~cur_occupied;
    move_set.add_piece_moves(Piece::Queen, queen, to_bitmap);
  }
}

void Board::generate_rook_moves(MoveSet& move_set) const {
  u64 rooks = cur_player()[Piece::Rook];
  u64 total_occupied = cur_occupied | opp_occupied;
  while (rooks) {
    u64 rook = bit::lsb(rooks);
    rooks ^= rook;
    u64 to_bitmap = bitboard::rook_attacks(rook, total_occupied) & ~cur_occupied;
    move_set.add_piece_moves(Piece::Rook, rook, to_bitmap);
  }
}

template <Board::MoveType MT>
void Board::generate_unchecked_rooklike_moves(std::vector<Move>& moves, u64 pinned_pieces) const {
  // Rooks that are pinned along a bishop ray cannot move at all.
  // Rooks that are pinned along a rook ray can only move along that ray.
  // Un-pinned rooks can move anywhere.
  const u64 total_occupied = cur_occupied | opp_occupied;
  const u64 king_rook_rays = bitboard::rook_attacks(cur_player()[Piece::King], total_occupied);
  const u64 opp_blockers = king_rook_rays & opp_occupied;  // Opponent pieces that block bishop rays from the king.
  u64 to_mask = bitboard::ALL;
  if constexpr (MT == MoveType::CapturesAndPromotionsOnly) to_mask = opp_occupied;
  if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
    to_mask = opp_occupied | bitboard::rook_attacks(opp_player()[Piece::King], total_occupied);
  }

  for (Piece from_piece : {Piece::Rook, Piece::Queen}) {
    if constexpr (MT == MoveType::CapturesChecksAndPromotionsOnly) {
      if (from_piece == Piece::Queen) to_mask |= bitboard::queen_attacks(opp_player()[Piece::King], total_occupied);
    }
    const u64 unpinned_pieces = cur_player()[from_piece] & ~pinned_pieces;
    bitboard::iterate(unpinned_pieces, [&](const u64 from) {
      const u64 to_bitboard = bitboard::rook_attacks(from, total_occupied) & ~cur_occupied & to_mask;
      bitboard::iterate(to_bitboard, [&](const u64 to) { moves.emplace_back(from_piece, from, to); });
    });

    const u64 pinned_from_pieces = king_rook_rays & pinned_pieces & cur_player()[from_piece];
    bitboard::iterate(pinned_from_pieces, [&](const u64 from) {
      const u64 pinned_by = bitboard::rook_attacks(cur_player()[Piece::King], total_occupied ^ from) &
                            (opp_player()[Piece::Rook] | opp_player()[Piece::Queen]) & ~opp_blockers;
      if (pinned_by == 0) return;
      const u64 to_bitboard =
          (bitboard::block_slider_check(cur_player()[Piece::King], pinned_by) | pinned_by) & ~from & to_mask;
      bitboard::iterate(to_bitboard, [&](const u64 to) { moves.emplace_back(from_piece, from, to); });
    });
  }
}

void Board::generate_king_single_check_evasions(std::vector<Move>& moves, u64 attacker) const {
  // To evade a single check, one of the following must be done:
  // 1. Capture the attacker with a piece that is not pinned.
  // 2. King moves to a square that is not attacked.
  // 3. Block the check if it is a sliding check.

  const Player& cur = cur_player();
  const Player& opp = opp_player();
  const u64 total_occupied = cur_occupied | opp_occupied;
  const u64 pinned_pieces = get_pinned_pieces();

  // 1. Capture the attacker with a piece that is not pinned.
  const u64 attacker_bishop_rays = bitboard::bishop_attacks(attacker, total_occupied);
  const u64 attacker_rook_rays = bitboard::rook_attacks(attacker, total_occupied);
  // Capture the attacker with a bishop that is not pinned.
  u64 bishop_capturers = attacker_bishop_rays & cur[Piece::Bishop] & ~pinned_pieces;
  bitboard::iterate(bishop_capturers,
                    [attacker, &moves](u64 from) { moves.emplace_back(Piece::Bishop, from, attacker); });
  // Capture the attacker with a knight that is not pinned.
  u64 knight_capturers = bitboard::knight_attacks(attacker) & cur[Piece::Knight] & ~pinned_pieces;
  bitboard::iterate(knight_capturers,
                    [attacker, &moves](u64 from) { moves.emplace_back(Piece::Knight, from, attacker); });
  // Capture the attacker with a pawn that is not pinned.
  u64 pawn_capturers = bitboard::pawn_attacks(attacker, !is_white_turn) & cur[Piece::Pawn] & ~pinned_pieces;
  bitboard::iterate(pawn_capturers, [this, attacker, &moves](u64 from) { add_pawn_moves(moves, from, attacker); });
  // Special en-passant check for pawns.
  if (attacker == (is_white_turn ? en_passant_bit >> 8 : en_passant_bit << 8)) {
    u64 capturing_square = is_white_turn ? attacker << 8 : attacker >> 8;
    u64 pawn_capturers = bitboard::pawn_attacks(capturing_square, !is_white_turn) & cur[Piece::Pawn] & ~pinned_pieces;
    bitboard::iterate(pawn_capturers, [capturing_square, &moves](u64 from) {
      moves.emplace_back(Piece::Pawn, from, capturing_square);
    });
  }
  // Capture the attacker with a queen that is not pinned.
  u64 queen_capturers = (attacker_bishop_rays | attacker_rook_rays) & cur[Piece::Queen] & ~pinned_pieces;
  bitboard::iterate(queen_capturers,
                    [attacker, &moves](u64 from) { moves.emplace_back(Piece::Queen, from, attacker); });
  // Capture the attacker with a rook that is not pinned.
  u64 rook_capturers = attacker_rook_rays & cur[Piece::Rook] & ~pinned_pieces;
  bitboard::iterate(rook_capturers, [attacker, &moves](u64 from) { moves.emplace_back(Piece::Rook, from, attacker); });

  // 2. King moves to a square that is not attacked. This is the same as evading double check.
  generate_king_double_check_evasions<MoveType::All>(moves);

  // 3. Block the check if it is a sliding check.
  const Piece attacker_piece = opp.piece_at(attacker);
  if (piece::is_slider(attacker_piece)) {
    const u64 blocking_squares = bitboard::block_slider_check(cur[Piece::King], attacker);
    bitboard::iterate(blocking_squares, [this, &moves, pinned_pieces, total_occupied](const u64 to) {
      const u64 blocker_bishop_rays = bitboard::bishop_attacks(to, total_occupied);
      const u64 blocker_rook_rays = bitboard::rook_attacks(to, total_occupied);
      // Block with a bishop that is not pinned.
      const u64 bishop_blockers = blocker_bishop_rays & cur_player()[Piece::Bishop] & ~pinned_pieces;
      bitboard::iterate(bishop_blockers, [&moves, to](const u64 from) { moves.emplace_back(Piece::Bishop, from, to); });
      // Block with a knight that is not pinned.
      const u64 knight_blockers = bitboard::knight_attacks(to) & cur_player()[Piece::Knight] & ~pinned_pieces;
      bitboard::iterate(knight_blockers, [&moves, to](const u64 from) { moves.emplace_back(Piece::Knight, from, to); });
      // Block with a pawn that is not pinned.
      u64 pawn_blockers = (is_white_turn ? to >> 8 : to << 8);
      if (is_white_turn) {
        pawn_blockers |= ((to >> 8) & ~total_occupied) >> 8 & bitboard::RANK_2;
      } else {
        pawn_blockers |= ((to << 8) & ~total_occupied) << 8 & bitboard::RANK_7;
      }
      pawn_blockers = pawn_blockers & cur_player()[Piece::Pawn] & ~pinned_pieces;
      bitboard::iterate(pawn_blockers, [&moves, to](const u64 from) { moves.emplace_back(Piece::Pawn, from, to); });
      // Block with a queen that is not pinned.
      const u64 queen_blockers =
          (blocker_bishop_rays | blocker_rook_rays) & cur_player()[Piece::Queen] & ~pinned_pieces;
      bitboard::iterate(queen_blockers, [&moves, to](const u64 from) { moves.emplace_back(Piece::Queen, from, to); });
      // Block with a rook that is not pinned.
      const u64 rook_blockers = blocker_rook_rays & cur_player()[Piece::Rook] & ~pinned_pieces;
      bitboard::iterate(rook_blockers, [&moves, to](const u64 from) { moves.emplace_back(Piece::Rook, from, to); });
    });
  }
}

template <Board::MoveType MT>
void Board::generate_king_double_check_evasions(std::vector<Move>& moves) const {
  // To evade a double check, the king must move to a square that is not attacked.
  const u64 king = cur_player()[Piece::King];
  u64 to_bitboard = bitboard::king_attacks(king) & ~cur_occupied;
  if constexpr (MT != MoveType::All) to_bitboard &= opp_occupied;
  const u64 total_occupied = (cur_occupied | opp_occupied) ^ king;
  bitboard::iterate(to_bitboard, [this, king, total_occupied, &moves](const u64 to) {
    const Player& opp = opp_player();
    if (bitboard::bishop_attacks(to, total_occupied) & (opp[Piece::Bishop] | opp[Piece::Queen])) return;
    if (bitboard::king_attacks(to) & opp[Piece::King]) return;
    if (bitboard::knight_attacks(to) & opp[Piece::Knight]) return;
    if (bitboard::pawn_attacks(to, is_white_turn) & opp[Piece::Pawn]) return;
    if (bitboard::rook_attacks(to, total_occupied) & (opp[Piece::Rook] | opp[Piece::Queen])) return;
    moves.emplace_back(Piece::King, king, to);
  });
}
