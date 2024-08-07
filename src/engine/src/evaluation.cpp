#include "evaluation.h"

#include <algorithm>
#include <array>
#include <cstdint>

// Values for the Piece Square Tables are taken from
// https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function

// Middlegame PST values indexed by [piece][position].
constexpr std::array<std::array<int16_t, 64>, 6> middlegame_values{[]() {
  using chess::PieceType;
  std::array<std::array<int16_t, 64>, 6> values;
  // These values are written from white's perspective of the chessboard for
  // easier reading, but are flipped in terms of y-coordinate.
  values[static_cast<size_t>(PieceType::Bishop)] = {{
      -29, 4,  -82, -37, -25, -42, 7,   -8,   //
      -26, 16, -18, -13, 30,  59,  18,  -47,  //
      -16, 37, 43,  40,  35,  50,  37,  -2,   //
      -4,  5,  19,  50,  37,  37,  7,   -2,   //
      -6,  13, 13,  26,  34,  12,  10,  4,    //
      0,   15, 15,  15,  14,  27,  18,  10,   //
      4,   15, 16,  0,   7,   21,  33,  1,    //
      -33, -3, -14, -21, -13, -12, -39, -21,  //
  }};
  values[static_cast<size_t>(PieceType::King)] = {{
      -65, 23,  16,  -15, -56, -34, 2,   13,   //
      29,  -1,  -20, -7,  -8,  -4,  -38, -29,  //
      -9,  24,  2,   -16, -20, 6,   22,  -22,  //
      -17, -20, -12, -27, -30, -25, -14, -36,  //
      -49, -1,  -27, -39, -46, -44, -33, -51,  //
      -14, -14, -22, -46, -44, -30, -15, -27,  //
      1,   7,   -8,  -64, -43, -16, 9,   8,    //
      -15, 36,  12,  -54, 8,   -28, 24,  14,   //
  }};
  values[static_cast<size_t>(PieceType::Knight)] = {{
      -167, -89, -34, -49, 61,  -97, -15, -107,  //
      -73,  -41, 72,  36,  23,  62,  7,   -17,   //
      -47,  60,  37,  65,  84,  129, 73,  44,    //
      -9,   17,  19,  53,  37,  69,  18,  22,    //
      -13,  4,   16,  13,  28,  19,  21,  -8,    //
      -23,  -9,  12,  10,  19,  17,  25,  -16,   //
      -29,  -53, -12, -3,  -1,  18,  -14, -19,   //
      -105, -21, -58, -33, -17, -28, -19, -23,   //
  }};
  values[static_cast<size_t>(PieceType::Pawn)] = {{
      0,   0,   0,   0,   0,   0,   0,  0,    //
      98,  134, 61,  95,  68,  126, 34, -11,  //
      -6,  7,   26,  31,  65,  56,  25, -20,  //
      -14, 13,  6,   21,  23,  12,  17, -23,  //
      -27, -2,  -5,  12,  17,  6,   10, -25,  //
      -26, -4,  -4,  -10, 3,   3,   33, -12,  //
      -35, -1,  -20, -23, -15, 24,  38, -22,  //
      0,   0,   0,   0,   0,   0,   0,  0,    //
  }};
  values[static_cast<size_t>(PieceType::Queen)] = {{
      -28, 0,   29,  12,  59,  44,  43,  45,   //
      -24, -39, -5,  1,   -16, 57,  28,  54,   //
      -13, -17, 7,   8,   29,  56,  47,  57,   //
      -27, -27, -16, -16, -1,  17,  -2,  1,    //
      -9,  -26, -9,  -10, -2,  -4,  3,   -3,   //
      -14, 2,   -11, -2,  -5,  2,   14,  5,    //
      -35, -8,  11,  2,   8,   15,  -3,  1,    //
      -1,  -18, -9,  10,  -15, -25, -31, -50,  //
  }};
  values[static_cast<size_t>(PieceType::Rook)] = {{
      32,  42,  32,  51,  63, 9,  31,  43,   //
      27,  32,  58,  62,  80, 67, 26,  44,   //
      -5,  19,  26,  36,  17, 45, 61,  16,   //
      -24, -11, 7,   26,  24, 35, -8,  -20,  //
      -36, -26, -12, -1,  9,  -7, 6,   -23,  //
      -45, -25, -16, -17, 3,  0,  -5,  -33,  //
      -44, -16, -20, -9,  -1, 11, -6,  -71,  //
      -19, -13, 1,   17,  16, 7,  -37, -26,  //
  }};

  std::array<int16_t, 6> piece_values;
  piece_values[static_cast<size_t>(PieceType::Bishop)] = 365;
  piece_values[static_cast<size_t>(PieceType::King)] = 0;
  piece_values[static_cast<size_t>(PieceType::Knight)] = 337;
  piece_values[static_cast<size_t>(PieceType::Pawn)] = 82;
  piece_values[static_cast<size_t>(PieceType::Queen)] = 1025;
  piece_values[static_cast<size_t>(PieceType::Rook)] = 477;
  for (size_t i{0}; i < piece_values.size(); i++) {
    // Add piece value to all squares.
    std::ranges::transform(values[i], values[i].begin(), [&](int16_t value) { return value + piece_values[i]; });
  }

  return values;
}()};

// Endgame PST values indexed by [piece][position].
constexpr std::array<std::array<int16_t, 64>, 6> endgame_values{[]() {
  using chess::PieceType;
  std::array<std::array<int16_t, 64>, 6> values;
  // These values are written from white's perspective of the chessboard for
  // easier reading, but are flipped in terms of y-coordinate.
  values[static_cast<size_t>(PieceType::Bishop)] = {{
      -14, -21, -11, -8,  -7, -9,  -17, -24,  //
      -8,  -4,  7,   -12, -3, -13, -4,  -14,  //
      2,   -8,  0,   -1,  -2, 6,   0,   4,    //
      -3,  9,   12,  9,   14, 10,  3,   2,    //
      -6,  3,   13,  19,  7,  10,  -3,  -9,   //
      -12, -3,  8,   10,  13, 3,   -7,  -15,  //
      -14, -18, -7,  -1,  4,  -9,  -15, -27,  //
      -23, -9,  -23, -5,  -9, -16, -5,  -17,  //
  }};
  values[static_cast<size_t>(PieceType::King)] = {{
      -74, -35, -18, -18, -11, 15,  4,   -17,  //
      -12, 17,  14,  17,  17,  38,  23,  11,   //
      10,  17,  23,  15,  20,  45,  44,  13,   //
      -8,  22,  24,  27,  26,  33,  26,  3,    //
      -18, -4,  21,  24,  27,  23,  9,   -11,  //
      -19, -3,  11,  21,  23,  16,  7,   -9,   //
      -27, -11, 4,   13,  14,  4,   -5,  -17,  //
      -53, -34, -21, -11, -28, -14, -24, -43   //
  }};
  values[static_cast<size_t>(PieceType::Knight)] = {{
      -58, -38, -13, -28, -31, -27, -63, -99,  //
      -25, -8,  -25, -2,  -9,  -25, -24, -52,  //
      -24, -20, 10,  9,   -1,  -9,  -19, -41,  //
      -17, 3,   22,  22,  22,  11,  8,   -18,  //
      -18, -6,  16,  25,  16,  17,  4,   -18,  //
      -23, -3,  -1,  15,  10,  -3,  -20, -22,  //
      -42, -20, -10, -5,  -2,  -20, -23, -44,  //
      -29, -51, -23, -15, -22, -18, -50, -64,  //
  }};
  values[static_cast<size_t>(PieceType::Pawn)] = {{
      0,   0,   0,   0,   0,   0,   0,   0,    //
      178, 173, 158, 134, 147, 132, 165, 187,  //
      94,  100, 85,  67,  56,  53,  82,  84,   //
      32,  24,  13,  5,   -2,  4,   17,  17,   //
      13,  9,   -3,  -7,  -7,  -8,  3,   -1,   //
      4,   7,   -6,  1,   0,   -5,  -1,  -8,   //
      13,  8,   8,   10,  13,  0,   2,   -7,   //
      0,   0,   0,   0,   0,   0,   0,   0,    //
  }};
  values[static_cast<size_t>(PieceType::Queen)] = {{
      -9,  22,  22,  27,  27,  19,  10,  20,   //
      -17, 20,  32,  41,  58,  25,  30,  0,    //
      -20, 6,   9,   49,  47,  35,  19,  9,    //
      3,   22,  24,  45,  57,  40,  57,  36,   //
      -18, 28,  19,  47,  31,  34,  39,  23,   //
      -16, -27, 15,  6,   9,   17,  10,  5,    //
      -22, -23, -30, -16, -16, -23, -36, -32,  //
      -33, -28, -22, -43, -5,  -32, -20, -41,  //
  }};
  values[static_cast<size_t>(PieceType::Rook)] = {{
      13, 10, 18, 15, 12, 12,  8,   5,    //
      11, 13, 13, 11, -3, 3,   8,   3,    //
      7,  7,  7,  5,  4,  -3,  -5,  -3,   //
      4,  3,  13, 1,  2,  1,   -1,  2,    //
      3,  5,  8,  4,  -5, -6,  -8,  -11,  //
      -4, 0,  -5, -1, -7, -12, -8,  -16,  //
      -6, -6, 0,  2,  -9, -9,  -11, -3,   //
      -9, 2,  3,  -1, -5, -13, 4,   -20,  //
  }};

  std::array<int16_t, 6> piece_values;
  piece_values[static_cast<size_t>(PieceType::Bishop)] = 297;
  piece_values[static_cast<size_t>(PieceType::King)] = 0;
  piece_values[static_cast<size_t>(PieceType::Knight)] = 281;
  piece_values[static_cast<size_t>(PieceType::Pawn)] = 94;
  piece_values[static_cast<size_t>(PieceType::Queen)] = 936;
  piece_values[static_cast<size_t>(PieceType::Rook)] = 512;
  for (size_t i{0}; i < piece_values.size(); i++) {
    // Add piece value to all squares.
    std::ranges::transform(values[i], values[i].begin(), [&](int16_t value) { return value + piece_values[i]; });
  }

  return values;
}()};

constexpr int16_t piece_phase_value[6]{1, 0, 1, 0, 4, 2};
constexpr int16_t total_phase = []() {
  using chess::PieceType;
  int16_t value{0};
  value += piece_phase_value[static_cast<int>(PieceType::Bishop)] * 4;
  value += piece_phase_value[static_cast<int>(PieceType::Knight)] * 4;
  value += piece_phase_value[static_cast<int>(PieceType::Pawn)] * 16;
  value += piece_phase_value[static_cast<int>(PieceType::Queen)] * 2;
  value += piece_phase_value[static_cast<int>(PieceType::Rook)] * 4;
  return value;
}();

Evaluation Evaluation::evaluate(const chess::Board &board) {
  using chess::PieceType;
  int16_t game_phase{total_phase};
  int16_t middlegame_evaluation{0};
  int16_t endgame_evaluation{0};

  for (const PieceType piece :
       {PieceType::Bishop, PieceType::Knight, PieceType::Pawn, PieceType::Queen, PieceType::Rook}) {
    const chess::Bitboard white_pieces{board.get_player<chess::Color::White>()[piece]};
    const chess::Bitboard black_pieces{board.get_player<chess::Color::Black>()[piece]};
    const size_t piece_index{static_cast<size_t>(piece)};
    game_phase -= (white_pieces.count() + black_pieces.count()) * piece_phase_value[piece_index];
    for (const chess::Bitboard bit : white_pieces.iterate()) {
      const int index{bit.to_index()};
      // ^ 56 flips the y-coordinate (e.g. y = 1 -> y = 6).
      middlegame_evaluation += middlegame_values[piece_index][index ^ 56];
      endgame_evaluation += endgame_values[piece_index][index ^ 56];
    }
    for (const chess::Bitboard bit : black_pieces.iterate()) {
      const int index{bit.to_index()};
      middlegame_evaluation -= middlegame_values[piece_index][index];
      endgame_evaluation -= endgame_values[piece_index][index];
    }
  }
  game_phase = (game_phase * 256 + (total_phase / 2)) / total_phase;
  const int16_t evaluation = ((middlegame_evaluation * (256 - game_phase)) + endgame_evaluation * game_phase) / 256;
  return Evaluation{board.is_white_to_move() ? evaluation : static_cast<int16_t>(-evaluation)};
}

Evaluation Evaluation::winning(int32_t depth) { return Evaluation{static_cast<int16_t>(20'000 + depth)}; }

Evaluation Evaluation::losing(int32_t depth) { return Evaluation{static_cast<int16_t>(-20'000 - depth)}; }

bool Evaluation::is_winning() const { return value >= 10'000; }

bool Evaluation::is_losing() const { return value <= -10'000; }

Evaluation Evaluation::succ() const { return Evaluation{static_cast<int16_t>(value + 1)}; };

int16_t Evaluation::to_centipawns() const { return value; }