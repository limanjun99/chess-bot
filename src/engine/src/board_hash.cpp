#include "board_hash.h"

#include <array>
#include <random>

namespace zobrist {
std::mt19937_64 gen64{0};

const std::array<std::array<std::array<u64, 64>, 6>, 2> piece_square_rng = []() {
  std::array<std::array<std::array<u64, 64>, 6>, 2> values;
  for (int color = 0; color < 2; color++) {
    for (int piece = 0; piece < 6; piece++) {
      for (int index = 0; index < 64; index++) {
        values[color][piece][index] = gen64();
      }
    }
  }
  return values;
}();

const u64 is_white_rng{gen64()};
const std::array<u64, 2> castle_kingside_rng{gen64(), gen64()};
const std::array<u64, 2> castle_queenside_rng{gen64(), gen64()};
const std::array<u64, 8> en_passant_file_rng{gen64(), gen64(), gen64(), gen64(), gen64(), gen64(), gen64(), gen64()};
}  // namespace zobrist

u64 board_hash::hash(const Board& board) {
  u64 hash = 0;
  if (board.is_white_to_move()) hash = zobrist::is_white_rng;
  if (board.get_en_passant()) hash ^= zobrist::en_passant_file_rng[bit::to_index(board.get_en_passant()) % 8];
  for (int is_white = 0; is_white < 2; is_white++) {
    const Player& player = is_white ? board.get_white() : board.get_black();
    if (player.can_castle_kingside()) hash ^= zobrist::castle_kingside_rng[is_white];
    if (player.can_castle_queenside()) hash ^= zobrist::castle_queenside_rng[is_white];
    for (int piece = 0; piece < 6; piece++) {
      u64 bitboard = player[static_cast<PieceVariant>(piece)];
      BITBOARD_ITERATE(bitboard, bit) { hash ^= zobrist::piece_square_rng[is_white][piece][bit::to_index(bit)]; }
    }
  }
  return hash;
}