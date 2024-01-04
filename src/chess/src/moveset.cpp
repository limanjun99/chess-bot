#include "moveset.h"

#include "board.h"

MoveSet::MoveSet(const Board& board) : board{board}, size{0}, promotion_flag{3} {}

void MoveSet::add_piece_moves(Piece piece, u64 from, u64 to_bitboard) {
  piece_moves[size] = {piece, from, to_bitboard};
  size++;
}

std::optional<std::pair<Move, Board>> MoveSet::apply_next() {
  while (size) {
    int index = size - 1;
    if (!piece_moves[index].to_bitboard) {
      size--;
      continue;
    }

    u64 from = piece_moves[index].from;
    u64 to = bit::lsb(piece_moves[index].to_bitboard);
    Piece piece = piece_moves[index].piece;
    piece_moves[index].to_bitboard ^= to;

    if (promotion_flag != 3 || (piece == Piece::Pawn && (to & (bitboard::RANK_1 | bitboard::RANK_8)))) {
      // Promotion.
      constexpr Piece promotion_pieces[4] = {Piece::Bishop, Piece::Knight, Piece::Queen, Piece::Rook};
      Piece promotion_piece = promotion_pieces[promotion_flag];
      promotion_flag = promotion_flag == 0 ? 3 : promotion_flag - 1;
      if (promotion_flag != 3) piece_moves[index].to_bitboard ^= to;
      Board new_board = board.apply_promotion(from, to, promotion_piece);
      if (new_board.moved_into_check()) continue;
      return {{Move{from, to, promotion_piece}, new_board}};
    }
    Board new_board = board.apply_move(piece, from, to);
    if (new_board.moved_into_check()) continue;
    return {{Move{piece, from, to}, new_board}};
  }
  return std::nullopt;
}
